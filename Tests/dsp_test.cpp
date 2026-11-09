#include "DeesserDSP.h"
#include <iostream>
#include <stdexcept>
#include <random>

using namespace deess01;
void expect(bool v,const char* why){if(!v)throw std::runtime_error(why);}
Settings basic(){Settings s;s.detector=1;s.lookahead=0;s.bands[0]={6000,-35,3,9,true};s.bands[1].enabled=false;s.ratio=8;s.attack=.1f;s.release=45;return s;}
struct Probe {double low=0,high=0,energy=0;float reduction=0;};
Probe render(Settings s,double fs=48000,int block=256) {
    Engine e;e.prepare(fs);Probe p;const int blocks=static_cast<int>(fs/block);double ls=0,lc=0,hs=0,hc=0;
    std::vector<float> l(block),r(block);int count=0;
    for(int b=0;b<blocks;++b){for(int i=0;i<block;++i){const int n=b*block+i;l[i]=r[i]=.1f*std::sin(2*pi*200*n/fs)+.2f*std::sin(2*pi*6000*n/fs);}e.process(l.data(),r.data(),block,s);
        if(b>blocks/2){for(int i=0;i<block;++i){const int n=b*block+i;ls+=l[i]*std::sin(2*pi*200*n/fs);lc+=l[i]*std::cos(2*pi*200*n/fs);hs+=l[i]*std::sin(2*pi*6000*n/fs);hc+=l[i]*std::cos(2*pi*6000*n/fs);p.energy+=l[i]*l[i];++count;}p.reduction=std::max(p.reduction,e.getMeter().reduction[0]);}
    }p.low=2*std::hypot(ls,lc)/count;p.high=2*std::hypot(hs,hc)/count;return p;
}
int main(){try{
    for(double fs:{44100.,48000.,96000.,192000.})for(int block:{1,32,257,4096}){
        Engine e;e.prepare(fs);Settings s;std::vector<float> l(block),r(block);
        for(int b=0;b<5;++b){e.process(l.data(),r.data(),block,s);for(float x:l)expect(x==0,"silence emits signal");}
        for(int lookahead=0;lookahead<3;++lookahead){e.reset();s.lookahead=lookahead;s.bands[0].range=s.bands[1].range=0;const int delay=e.latency(s);std::vector<float> a(delay+32),b(delay+32);a[0]=.4f;b[0]=-.2f;e.process(a.data(),b.data(),static_cast<int>(a.size()),s);for(size_t i=0;i<a.size();++i){expect(std::abs(a[i]-(i==static_cast<size_t>(delay)?.4f:0.f))<1.e-6,"lookahead dry alignment wrong");expect(std::abs(b[i]-(i==static_cast<size_t>(delay)?-.2f:0.f))<1.e-6,"stereo alignment wrong");}}
    }
    auto s=basic();const auto split=render(s);s.mode=1;const auto wide=render(s);
    expect(split.high<.1,"split does not suppress sibilance");expect(std::abs(split.low-.1)<.0015,"split damages vocal body");expect(wide.low<.06,"wide does not attenuate full band");expect(split.reduction<=9.001f&&split.reduction>7,"range cap failed");
    s=basic();s.bands[0].range=0;const auto noRange=render(s);expect(std::abs(noRange.high-.2)<.001,"zero range is not transparent");
    s=basic();s.mix=0;const auto dry=render(s);expect(std::abs(dry.low-.1)<.001&&std::abs(dry.high-.2)<.001,"dry mix changed signal");
    s=basic();s.listen=2;const auto removed=render(s);expect(removed.low<.003&&removed.high>.09,"removed monitor wrong");
    s=basic();s.listen=1;const auto detector=render(s);expect(detector.high>detector.low*40,"detector monitor not band limited");
    // Adaptive level tracking should give a similar reduction when voice gain changes.
    s=basic();s.detector=0;auto adaptive=render(s);expect(adaptive.high<.13&&adaptive.reduction>3,"adaptive detector fails on sibilants");
    for(int channel=1;channel<=2;++channel){Engine e;e.prepare(48000);s=basic();s.channel=channel;std::vector<float> l(48000),r(48000);for(int i=0;i<48000;++i){l[i]=.2f*std::sin(2*pi*6000*i/48000);r[i]=channel==1?-l[i]:l[i];}auto original=l;e.process(l.data(),r.data(),48000,s);for(int i=24000;i<48000;++i)expect(std::abs(l[i]-original[i])<1.e-5,"Mid/Side processing changes the excluded component");}
    // Host bypass remains delayed and independent of output gain or audition.
    {Engine e;e.prepare(48000);s=basic();s.bypass=true;s.output=12;s.listen=2;s.lookahead=2;std::vector<float> l(2000,.15f),r(2000,-.1f);e.process(l.data(),r.data(),2000,s);for(int i=e.latency(s);i<2000;++i)expect(std::abs(l[i]-.15f)<1.e-6&&std::abs(r[i]+.1f)<1.e-6,"bypass altered audio");}
    {Engine e;e.prepare(48000);s=basic();std::vector<float> l(256),r(256);std::mt19937 random(42);std::uniform_real_distribution<float> noise(-.5f,.5f);for(int block=0;block<600;++block){s.bands[0].frequency=2500+(block%80)*140;s.bands[1].q=.35f+(block%50)*.1f;s.channel=block/100%3;s.listen=block/60%3;s.mode=block/50%2;s.detector=block/40%2;for(int i=0;i<256;++i)l[i]=r[i]=noise(random);e.process(l.data(),r.data(),256,s);for(float x:l)expect(std::isfinite(x)&&std::abs(x)<5,"automation causes unstable output");}e.process(l.data(),nullptr,0,s);}
    std::cout<<"PASS: silence, latency/bypass alignment, 4 sample rates, 4 block sizes, selective suppression and preserved body, wideband action, range cap, mix, detector/removed monitoring, adaptive detection, M/S isolation, automation stability\n";
    std::cout<<"Split: low="<<gainToDb(static_cast<float>(split.low/.1))<<" dB, sibilance="<<gainToDb(static_cast<float>(split.high/.2))<<" dB; wide body="<<gainToDb(static_cast<float>(wide.low/.1))<<" dB\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
