#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
void holdPeak(std::atomic<float>& meter,float value) {
    auto old=meter.load(std::memory_order_relaxed);
    while(old<value&&!meter.compare_exchange_weak(old,value,std::memory_order_relaxed)){}
}
}

juce::AudioProcessorValueTreeState::ParameterLayout DeessProcessor::layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto number=[&](const char* id,const char* name,float low,float high,float initial,float skew,const char* unit){
        auto range=juce::NormalisableRange<float>(low,high,0,skew);
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,range,initial,juce::AudioParameterFloatAttributes().withLabel(unit)));
    };
    number("b1freq","Frequency",2500,14000,6000,.45f,"Hz");
    number("b1threshold","Threshold",-54,0,-25,1,"dB");
    number("b1range","Range",0,15,6,1,"dB");
    number("b1q","Focus",.5f,6,2.2f,.5f,"Q");
    number("ratio","Ratio",1,12,4,.5f,":1");
    number("attack","Attack",.05f,10,.35f,.35f,"ms");
    number("release","Release",15,250,65,.5f,"ms");
    number("mix","Mix",0,100,100,1,"%");
    number("output","Output",-12,12,0,1,"dB");
    auto choice=[&](const char* id,const char* name,juce::StringArray options,int initial){p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,options,initial));};
    choice("mode","Processing",{"Split","Wide"},0);
    choice("listen","Listen",{"Processed","Sibilance","Removed"},0);
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
    return p;
}
DeessProcessor::DeessProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"DEESS01",layout()) {}
float DeessProcessor::value(const juce::String& id) const {return parameters.getRawParameterValue(id)->load();}
void DeessProcessor::setValue(const juce::String& id,float v) {auto* p=parameters.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}
deess01::Settings DeessProcessor::settings() const {
    deess01::Settings s;
    s.bands[0]={value("b1freq"),value("b1threshold"),value("b1q"),value("b1range"),true};
    s.bands[1].enabled=false;
    s.ratio=value("ratio");s.attack=value("attack");s.release=value("release");s.mix=value("mix")*.01f;s.output=value("output");
    s.mode=static_cast<int>(value("mode"));s.detector=0;s.followLevel=false;s.channel=0;s.lookahead=0;s.listen=static_cast<int>(value("listen"));s.bypass=value("bypass")>.5f;
    return s;
}
void DeessProcessor::prepareToPlay(double sr,int) {engine.prepare(sr);setLatencySamples(engine.latency(settings()));samplePosition=0;for(auto& v:samples)v=0;for(auto& v:gainReduction)v=0;inputPeak=0;outputPeak=0;}
bool DeessProcessor::isBusesLayoutSupported(const BusesLayout& l) const {const auto in=l.getMainInputChannelSet(),out=l.getMainOutputChannelSet();return in==out&&(in==juce::AudioChannelSet::mono()||in==juce::AudioChannelSet::stereo());}
void DeessProcessor::process(juce::AudioBuffer<float>& b,bool bypassed) {
    juce::ScopedNoDenormals noDenormals;
    for(int c=getTotalNumInputChannels();c<b.getNumChannels();++c)b.clear(c,0,b.getNumSamples());
    if(b.getNumSamples()==0||b.getNumChannels()==0)return;
    auto s=settings();s.bypass=s.bypass||bypassed;
    const int latency=engine.latency(s);if(getLatencySamples()!=latency)setLatencySamples(latency);
    unsigned pos=samplePosition.load(std::memory_order_relaxed);
    for(int i=0;i<b.getNumSamples();++i) {const float x=b.getSample(0,i);samples[pos++&2047].store(std::isfinite(x)?x:0,std::memory_order_relaxed);}
    samplePosition.store(pos,std::memory_order_release);
    engine.process(b.getWritePointer(0),b.getNumChannels()>1?b.getWritePointer(1):nullptr,b.getNumSamples(),s);
    const auto& m=engine.getMeter();for(int i=0;i<2;++i)holdPeak(gainReduction[i],m.reduction[i]);
    holdPeak(inputPeak,m.input);holdPeak(outputPeak,m.output);
}
void DeessProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {process(b,false);}
void DeessProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {process(b,true);}
void DeessProcessor::getStateInformation(juce::MemoryBlock& data) {auto state=parameters.copyState();state.setProperty("detailsView",detailsView.load(),nullptr);auto xml=state.createXml();copyXmlToBinary(*xml,data);}
void DeessProcessor::setStateInformation(const void* data,int size) {auto xml=getXmlFromBinary(data,size);if(xml&&xml->hasTagName(parameters.state.getType())){auto state=juce::ValueTree::fromXml(*xml);detailsView=static_cast<bool>(state.getProperty("detailsView",false));parameters.replaceState(state);}}
juce::AudioProcessorEditor* DeessProcessor::createEditor() {return new DeessEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {return new DeessProcessor();}
