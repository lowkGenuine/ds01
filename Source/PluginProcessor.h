#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "DeesserDSP.h"

class DeessProcessor final : public juce::AudioProcessor {
public:
    DeessProcessor();
    juce::AudioProcessorValueTreeState parameters;
    std::array<std::atomic<float>,2> gainReduction{};
    std::atomic<float> inputPeak{0},outputPeak{0};
    std::array<std::atomic<float>,2048> samples{};
    std::atomic<unsigned> samplePosition{0};
    std::atomic<bool> detailsView{false};
    float value(const juce::String&) const;
    void setValue(const juce::String&,float);
    deess01::Settings settings() const;
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void reset() override {engine.reset();}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorParameter* getBypassParameter() const override {return parameters.getParameter("bypass");}
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "DEESS-01";}
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 0;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
private:
    deess01::Engine engine;
    void process(juce::AudioBuffer<float>&,bool);
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeessProcessor)
};
