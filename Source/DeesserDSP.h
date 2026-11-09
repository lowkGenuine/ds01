#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace deess01 {
constexpr double pi = 3.14159265358979323846;
inline float dbToGain(float v) { return std::pow(10.f, v * .05f); }
inline float gainToDb(float v) { return 20.f * std::log10(std::max(v, 1.e-10f)); }
struct BandSettings {
    float frequency = 5500, threshold = -25, q = 1.5f, range = 6;
    bool enabled = true;
};
struct Settings {
    std::array<BandSettings,2> bands{{{5500,-25,1.5f,6,true},{9000,-22,2,4,true}}};
    float ratio = 4, attack = .4f, release = 70, mix = 1, output = 0;
    int mode = 0;       // 0: selective split, 1: wideband
    int detector = 0;   // 0: level-adaptive and dominance-aware, 1: classic absolute
    int listen = 0;     // 0: processed, 1: detector, 2: removed signal
    int channel = 0;    // stereo linked, mid, side
    int lookahead = 1;  // 0, 1.5, 5 ms
    bool bypass = false;
    bool followLevel = true;
};
struct Meter { std::array<float,2> reduction{}; float input = 0, output = 0; };

struct Biquad {
    double b0=0,b2=0,a1=0,a2=0,z1=0,z2=0;
    void set(double fs,double freq,double q) {
        const double w=2*pi*std::clamp(freq,30.,fs*.45)/fs;
        const double alpha=std::sin(w)/(2*std::clamp(q,.35,6.));
        const double a0=1+alpha;
        b0=alpha/a0; b2=-b0; a1=-2*std::cos(w)/a0; a2=(1-alpha)/a0;
    }
    float tick(float x) {
        const double y=b0*x+z1;
        z1=-a1*y+z2; z2=b2*x-a2*y;
        if(std::abs(z1)<1.e-25)z1=0;
        if(std::abs(z2)<1.e-25)z2=0;
        return static_cast<float>(y);
    }
    void reset() { z1=z2=0; }
};

class Engine {
public:
    void prepare(double sampleRate) {
        fs=std::clamp(sampleRate,8000.,768000.);
        const int capacity=static_cast<int>(std::ceil(fs*.005))+2;
        for(auto& d:delay)d.assign(static_cast<size_t>(capacity),0);
        reset();
    }
    void reset() {
        for(auto& d:delay)std::fill(d.begin(),d.end(),0);
        write=0; counter=0; initialized=false;
        for(auto& band:detectorFilters)for(auto& f:band)f.reset();
        for(auto& band:audioFilters)for(auto& f:band)f.reset();
        for(auto& e:env)e.fill(0);
        for(auto& e:reduction)e.fill(0);
        full.fill(0); slow.fill(0);
        meters={};
    }
    int latency(const Settings& s) const {
        return static_cast<int>(std::round(fs*(s.lookahead==0?0:s.lookahead==1?.0015:.005)));
    }
    const Meter& getMeter() const { return meters; }
    void process(float* left,float* right,int n,const Settings& s) {
        if(delay[0].empty())prepare(48000);
        meters={};
        const int ahead=latency(s),capacity=static_cast<int>(delay[0].size());
        const float attack=coefficient(s.attack),release=coefficient(s.release);
        const float envAttack=coefficient(.15f),envRelease=coefficient(8);
        const float bodyAttack=coefficient(2),bodyRelease=coefficient(35);
        const float levelCoef=coefficient(100),control=coefficient(5);
        const float outputGain=dbToGain(s.output);
        const float detectorScale=1.f/std::max(1,static_cast<int>(s.bands[0].enabled)+static_cast<int>(s.bands[1].enabled));
        if(!initialized) {
            for(int b=0;b<2;++b){frequency[b]=s.bands[b].frequency;quality[b]=s.bands[b].q;}
            mode=static_cast<float>(s.mode); mix=s.mix; gain=outputGain; wet=s.bypass?0.f:1.f;
            channel.fill(0);channel[std::clamp(s.channel,0,2)]=1;
            listen.fill(0);listen[std::clamp(s.listen,0,2)]=1;
            initialized=true;
        }
        for(int i=0;i<n;++i) {
            const float l=std::isfinite(left[i])?left[i]:0;
            const float r=right?(std::isfinite(right[i])?right[i]:0):l;
            meters.input=std::max(meters.input,std::max(std::abs(l),std::abs(r)));
            delay[0][write]=l;delay[1][write]=r;
            const int read=(write-ahead+capacity)%capacity;
            const float dl=delay[0][read],dr=delay[1][read];
            write=(write+1)%capacity;
            const std::array<float,4> detectorInput{{l,r,(l+r)*.5f,(l-r)*.5f}};
            const std::array<float,3> body{{std::max(std::abs(l),std::abs(r)),std::abs(detectorInput[2]),std::abs(detectorInput[3])}};
            for(int c=0;c<3;++c) {
                full[c]=follow(full[c],body[c],bodyAttack,bodyRelease);
                slow[c]=levelCoef*slow[c]+(1-levelCoef)*body[c]*body[c];
            }
            for(int b=0;b<2;++b) {
                frequency[b]=control*frequency[b]+(1-control)*s.bands[b].frequency;
                quality[b]=control*quality[b]+(1-control)*s.bands[b].q;
            }
            if((counter++&15)==0)for(int b=0;b<2;++b) {
                for(auto& f:detectorFilters[b])f.set(fs,frequency[b],quality[b]);
                for(auto& f:audioFilters[b])f.set(fs,frequency[b],quality[b]);
            }
            std::array<float,4> detectorSum{};
            std::array<std::array<float,3>,2> gains{};
            for(int b=0;b<2;++b) {
                std::array<float,4> filtered{};
                for(int c=0;c<4;++c) {
                    filtered[c]=detectorFilters[b][c].tick(detectorInput[c]);
                    if(s.bands[b].enabled)detectorSum[c]+=filtered[c]*detectorScale;
                }
                const std::array<float,3> level{{std::max(std::abs(filtered[0]),std::abs(filtered[1])),std::abs(filtered[2]),std::abs(filtered[3])}};
                for(int c=0;c<3;++c) {
                    env[b][c]=follow(env[b][c],level[c],envAttack,envRelease);
                    float threshold=s.bands[b].threshold;
                    float dominance=1;
                    if(s.detector==0) {
                        if(s.followLevel)threshold+=std::clamp(gainToDb(std::sqrt(slow[c]))+18.f,-9.f,9.f);
                        const float proportion=env[b][c]/std::max(full[c],1.e-8f);
                        dominance=std::clamp((proportion-.12f)/.25f,0.f,1.f);
                        dominance=dominance*dominance*(3-2*dominance);
                    }
                    const float over=gainToDb(env[b][c])-threshold;
                    constexpr float knee=4;
                    const float above=over<=-knee*.5f?0:over>=knee*.5f?over:(over+knee*.5f)*(over+knee*.5f)/(2*knee);
                    const float target=s.bands[b].enabled?std::min(s.bands[b].range,above*(1-1/std::max(s.ratio,1.f))*dominance):0;
                    reduction[b][c]=follow(reduction[b][c],target,attack,release);
                    if(reduction[b][c]<1.e-8f)reduction[b][c]=0;
                    gains[b][c]=dbToGain(-reduction[b][c]);
                }
            }
            std::array<float,4> split{{dl,dr,(dl+dr)*.5f,(dl-dr)*.5f}};
            const auto delayed=split;
            for(int b=0;b<2;++b)for(int c=0;c<4;++c) {
                const int detectorChannel=c<2?0:c-1;
                const float part=audioFilters[b][c].tick(split[c]);
                split[c]+=(gains[b][detectorChannel]-1)*part;
            }
            mode=control*mode+(1-control)*static_cast<float>(s.mode);
            for(int c=0;c<4;++c) {
                const int detectorChannel=c<2?0:c-1;
                const float wide=delayed[c]*gains[0][detectorChannel]*gains[1][detectorChannel];
                split[c]=split[c]+mode*(wide-split[c]);
            }
            for(int c=0;c<3;++c) {
                channel[c]=control*channel[c]+(1-control)*(s.channel==c?1.f:0.f);
                listen[c]=control*listen[c]+(1-control)*(s.listen==c?1.f:0.f);
            }
            const float sl=channel[0]*split[0]+channel[1]*(split[2]+delayed[3])+channel[2]*(delayed[2]+split[3]);
            const float sr=channel[0]*split[1]+channel[1]*(split[2]-delayed[3])+channel[2]*(delayed[2]-split[3]);
            const float ml=channel[0]*detectorSum[0]+channel[1]*detectorSum[2]+channel[2]*detectorSum[3];
            const float mr=channel[0]*detectorSum[1]+channel[1]*detectorSum[2]-channel[2]*detectorSum[3];
            mix=control*mix+(1-control)*std::clamp(s.mix,0.f,1.f);
            gain=control*gain+(1-control)*outputGain;
            wet=control*wet+(1-control)*(s.bypass?0.f:1.f);
            // Monitor paths intentionally bypass the mix control for an unambiguous audition.
            const float pl=listen[0]*(dl+mix*(sl-dl))+listen[1]*ml+listen[2]*(dl-sl);
            const float pr=listen[0]*(dr+mix*(sr-dr))+listen[1]*mr+listen[2]*(dr-sr);
            left[i]=dl+wet*(pl*gain-dl);
            if(right)right[i]=dr+wet*(pr*gain-dr);
            meters.output=std::max(meters.output,std::max(std::abs(left[i]),right?std::abs(right[i]):0.f));
            for(int b=0;b<2;++b) {
                float gr=0;for(int c=0;c<3;++c)gr+=channel[c]*reduction[b][c];
                meters.reduction[b]=std::max(meters.reduction[b],gr);
            }
        }
    }
private:
    double fs=48000;
    std::array<std::vector<float>,2> delay;
    int write=0,counter=0;
    bool initialized=false;
    std::array<std::array<Biquad,4>,2> detectorFilters{},audioFilters{};
    std::array<std::array<float,3>,2> env{},reduction{};
    std::array<float,3> full{},slow{},channel{},listen{};
    std::array<float,2> frequency{},quality{};
    float mode=0,mix=1,gain=1,wet=1;
    Meter meters{};
    float coefficient(float ms) const {return std::exp(-1.f/(static_cast<float>(fs)*std::max(ms,.01f)*.001f));}
    static float follow(float old,float v,float attack,float release) {const float c=v>old?attack:release;return c*old+(1-c)*v;}
};
}
