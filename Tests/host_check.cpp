#include <juce_audio_utils/juce_audio_utils.h>
#include <iostream>
#include <stdexcept>
#include <array>

void expect(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
juce::AudioProcessorParameter* parameter(juce::AudioProcessor& p,const char* name){for(auto* v:p.getParameters())if(v->getName(100)==name)return v;throw std::runtime_error(std::string("Missing host parameter: ")+name);}
void set(juce::AudioProcessor& p,const char* name,float normalized){auto* v=parameter(p,name);v->beginChangeGesture();v->setValueNotifyingHost(normalized);v->endChangeGesture();}

std::array<double,2> render(juce::AudioProcessor& p,int mode) {
    constexpr double pi=3.14159265358979323846;
    set(p,"Processing",static_cast<float>(mode));set(p,"Range",6.f/15);set(p,"Bypass",0);set(p,"Listen",0);
    p.setPlayConfigDetails(2,2,48000,256);p.prepareToPlay(48000,256);
    juce::AudioBuffer<float> b(2,256);juce::MidiBuffer midi;double ls=0,lc=0,hs=0,hc=0;int count=0;
    for(int block=0;block<190;++block){for(int i=0;i<256;++i){const auto n=block*256+i;const float x=.1f*std::sin(2*pi*200*n/48000)+.2f*std::sin(2*pi*6000*n/48000);b.setSample(0,i,x);b.setSample(1,i,x*.7f);}p.processBlock(b,midi);
        if(block>95)for(int i=0;i<256;++i){const auto n=block*256+i;const float x=b.getSample(0,i);expect(std::isfinite(x),"Non-finite processed audio");expect(std::abs(b.getSample(1,i)-.7f*x)<1.e-5,"Stereo link changes the stereo balance");ls+=x*std::sin(2*pi*200*n/48000);lc+=x*std::cos(2*pi*200*n/48000);hs+=x*std::sin(2*pi*6000*n/48000);hc+=x*std::cos(2*pi*6000*n/48000);++count;}
    }
    expect(p.getLatencySamples()==0,"Unexpected plugin latency");p.releaseResources();return {2*std::hypot(ls,lc)/count,2*std::hypot(hs,hc)/count};
}
int main(int argc,char** argv){try{
    juce::ScopedJuceInitialiser_GUI gui;expect(argc>1,"VST3 bundle path required");
    juce::AudioPluginFormatManager formats;formats.addDefaultFormats();juce::KnownPluginList known;juce::OwnedArray<juce::PluginDescription> descriptions;
    for(auto* f:formats.getFormats())known.scanAndAddFile(juce::String::fromUTF8(argv[1]),true,descriptions,*f);
    expect(descriptions.size()==1&&descriptions[0]->name=="DEESS-01","VST3 discovery failed");
    juce::String error;auto p=formats.createPluginInstance(*descriptions[0],48000,256,error);if(!p)throw std::runtime_error(error.toStdString());
    for(const char* name:{"Frequency","Threshold","Range","Focus","Attack","Release","Ratio","Mix","Output","Processing","Listen","Bypass"})parameter(*p,name);
    juce::MidiBuffer midi;set(*p,"Range",0);
    for(double sr:{44100.,48000.,96000.,192000.})for(int block:{1,32,257,4096}) {
        p->setPlayConfigDetails(2,2,sr,block);p->prepareToPlay(sr,block);expect(p->getLatencySamples()==0,"Host latency is not zero");juce::AudioBuffer<float> b(2,block);
        for(int k=0;k<8;++k){for(int i=0;i<block;++i){b.setSample(0,i,.1f);b.setSample(1,i,-.2f);}p->processBlock(b,midi);for(int i=0;i<block;++i)expect(std::abs(b.getSample(0,i)-.1f)<2.e-6&&std::abs(b.getSample(1,i)+.2f)<2.e-6,"Zero Range is not transparent");}p->releaseResources();
    }
    const auto split=render(*p,0),wide=render(*p,1);
    expect(std::abs(split[0]-.1)<.002&&split[1]<.13,"Split does not isolate sibilance");expect(wide[0]<.07,"Wide does not turn down the full voice");
    set(*p,"Frequency",.67f);set(*p,"Focus",.4f);set(*p,"Listen",1);juce::MemoryBlock state;p->getStateInformation(state);
    set(*p,"Frequency",.1f);set(*p,"Focus",.1f);set(*p,"Listen",0);p->setStateInformation(state.getData(),static_cast<int>(state.getSize()));
    expect(std::abs(parameter(*p,"Frequency")->getValue()-.67f)<1.e-5&&std::abs(parameter(*p,"Focus")->getValue()-.4f)<1.e-5&&parameter(*p,"Listen")->getValue()>.999,"Host state restoration failed");
    set(*p,"Bypass",1);p->setPlayConfigDetails(2,2,48000,256);p->prepareToPlay(48000,256);juce::AudioBuffer<float> b(2,256);
    for(int i=0;i<256;++i){b.setSample(0,i,.12f);b.setSample(1,i,-.08f);}p->processBlock(b,midi);expect(std::abs(b.getSample(0,200)-.12f)<1.e-6,"Binary bypass does not preserve the input");p->releaseResources();
    {auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p->createEditor());expect(editor&&editor->getWidth()==840&&editor->getHeight()==300,"Spartan editor did not load");}
    auto layout=p->getBusesLayout();layout.inputBuses.getReference(0)=layout.outputBuses.getReference(0)=juce::AudioChannelSet::mono();expect(p->setBusesLayout(layout),"Mono layout rejected");p->setRateAndBufferSizeDetails(48000,256);p->prepareToPlay(48000,256);juce::AudioBuffer<float> mono(1,256);mono.clear();p->processBlock(mono,midi);juce::AudioBuffer<float> empty(1,0);p->processBlock(empty,midi);p->releaseResources();
    std::cout<<"PASS: compiled VST3 scan/load, 12 exposed controls, zero latency and transparent zero Range at 4 sample rates / 4 buffer sizes, selective and wideband suppression, stereo balance, saved settings, bypass, editor, mono and empty buffers\n";
    std::cout<<"Binary Split: body "<<20*std::log10(split[0]/.1)<<" dB, sibilance "<<20*std::log10(split[1]/.2)<<" dB\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
