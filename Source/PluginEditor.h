#pragma once
#include "PluginProcessor.h"
#include <juce_dsp/juce_dsp.h>

class SpartanSkin final : public juce::LookAndFeel_V4 {
public:
    SpartanSkin();
    void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,const juce::Slider::SliderStyle,juce::Slider&) override;
    juce::Slider::SliderLayout getSliderLayout(juce::Slider&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&,juce::Label&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override {return juce::Font("Segoe UI",12,juce::Font::plain);}
};

class ParameterControl final : public juce::Component {
public:
    ParameterControl(DeessProcessor&,const char* id,const char* title,const char* hint,bool compact);
    juce::Slider slider;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    juce::String title,hint;
    bool compact;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class Spectrum final : public juce::Component {
public:
    explicit Spectrum(DeessProcessor& owner):p(owner) {values.fill(-96);}
    void update();
    void paint(juce::Graphics&) override;
private:
    DeessProcessor& p;
    juce::dsp::FFT fft{10};
    std::array<float,2048> data{};
    std::array<float,512> values{};
    unsigned lastPosition=0;
    float xFor(float) const;
    float yFor(float) const;
};

class DeessEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit DeessEditor(DeessProcessor&);
    ~DeessEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void showDetails(bool);
private:
    DeessProcessor& p;
    SpartanSkin skin;
    Spectrum spectrum;
    juce::TooltipWindow tips{this,550};
    std::vector<std::unique_ptr<ParameterControl>> controls;
    juce::TextButton mainTab{"De-ess"},detailsTab{"Details"},bypass{"Active"};
    juce::ComboBox mode,listen;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment,listenAttachment;
    float reduction=0,inputLevel=0,outputLevel=0;
    int clipFrames=0;
    void timerCallback() override;
};
