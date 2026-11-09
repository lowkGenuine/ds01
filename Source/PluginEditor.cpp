#include "PluginEditor.h"

namespace {
const juce::Colour canvas{0xfffafafa},white{0xffffffff},ink{0xff18181b},muted{0xff71717a},border{0xffe4e4e7},soft{0xfff4f4f5};
void label(juce::Graphics& g,const juce::String& s,juce::Rectangle<int> r,float size=12,juce::Colour colour=ink,bool bold=false,juce::Justification align=juce::Justification::centredLeft) {
    g.setColour(colour);g.setFont(juce::Font("Segoe UI",size,bold?juce::Font::bold:juce::Font::plain));g.drawText(s,r,align,false);
}
}

SpartanSkin::SpartanSkin() {
    setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxBackgroundColourId,canvas);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxHighlightColourId,border);setColour(juce::ComboBox::textColourId,ink);
    setColour(juce::PopupMenu::backgroundColourId,white);setColour(juce::PopupMenu::textColourId,ink);setColour(juce::PopupMenu::highlightedBackgroundColourId,soft);setColour(juce::PopupMenu::highlightedTextColourId,ink);
    setColour(juce::TooltipWindow::backgroundColourId,white);setColour(juce::TooltipWindow::textColourId,ink);setColour(juce::TooltipWindow::outlineColourId,border);
    setColour(juce::TextEditor::backgroundColourId,white);setColour(juce::TextEditor::textColourId,ink);setColour(juce::TextEditor::highlightColourId,border);setColour(juce::TextEditor::highlightedTextColourId,ink);setColour(juce::TextEditor::focusedOutlineColourId,ink);
}
void SpartanSkin::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,float pos,float,float,const juce::Slider::SliderStyle,juce::Slider&) {
    const float cy=y+h*.5f;
    g.setColour(border);g.fillRoundedRectangle(static_cast<float>(x),cy-1.5f,static_cast<float>(w),3,1.5f);
    g.setColour(ink);g.fillRoundedRectangle(static_cast<float>(x),cy-1.5f,std::max(0.f,pos-x),3,1.5f);
    g.setColour(white);g.fillEllipse(pos-5,cy-5,10,10);g.setColour(ink);g.drawEllipse(pos-5,cy-5,10,10,1.5f);
}
juce::Slider::SliderLayout SpartanSkin::getSliderLayout(juce::Slider& s) {
    const bool compact=static_cast<bool>(s.getProperties()["compact"]);
    juce::Slider::SliderLayout l;
    l.textBoxBounds={0,0,s.getWidth(),compact?30:44};
    l.sliderBounds={5,compact?38:56,s.getWidth()-10,20};
    return l;
}
juce::Label* SpartanSkin::createSliderTextBox(juce::Slider& s) {
    auto* value=juce::LookAndFeel_V4::createSliderTextBox(s);
    value->setFont(juce::Font("Segoe UI",static_cast<bool>(s.getProperties()["compact"])?18.f:29.f,juce::Font::plain));
    value->setJustificationType(juce::Justification::centredLeft);value->setBorderSize(juce::BorderSize<int>(0));
    value->setColour(juce::Label::textColourId,ink);value->setColour(juce::Label::backgroundColourId,canvas);value->setColour(juce::Label::outlineColourId,juce::Colours::transparentBlack);
    return value;
}
void SpartanSkin::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down) {
    auto r=b.getLocalBounds().toFloat().reduced(.5f);
    const bool selected=b.getToggleState();const bool status=b.getName()=="Bypass";
    g.setColour(selected?ink:(hover||down)?soft:status?white:canvas);g.fillRoundedRectangle(r,5);
    if(status||b.hasKeyboardFocus(true)){g.setColour(b.hasKeyboardFocus(true)?ink:border);g.drawRoundedRectangle(r,5,1);}
}
void SpartanSkin::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool) {
    label(g,b.getButtonText(),b.getLocalBounds(),12,b.getToggleState()?white:ink,false,juce::Justification::centred);
}
void SpartanSkin::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox& c) {
    g.setColour(white);g.fillRoundedRectangle(.5f,.5f,w-1.f,h-1.f,5);
    g.setColour(c.hasKeyboardFocus(true)?ink:border);g.drawRoundedRectangle(.5f,.5f,w-1.f,h-1.f,5,1);
    juce::Path arrow;arrow.startNewSubPath(w-17.f,h*.44f);arrow.lineTo(w-13.f,h*.56f);arrow.lineTo(w-9.f,h*.44f);g.setColour(muted);g.strokePath(arrow,juce::PathStrokeType(1.2f));
}
void SpartanSkin::positionComboBoxText(juce::ComboBox& c,juce::Label& l) {l.setBounds(10,0,c.getWidth()-33,c.getHeight());l.setFont(getComboBoxFont(c));}

ParameterControl::ParameterControl(DeessProcessor& p,const char* id,const char* name,const char* help,bool small):title(name),hint(help),compact(small) {
    setName(name);slider.setName(name);slider.setTitle(name);slider.getProperties().set("compact",compact);
    slider.setSliderStyle(juce::Slider::LinearHorizontal);slider.setTextBoxStyle(juce::Slider::TextBoxAbove,false,166,44);slider.setScrollWheelEnabled(false);slider.setWantsKeyboardFocus(true);
    slider.setColour(juce::Slider::textBoxTextColourId,ink);slider.setColour(juce::Slider::textBoxBackgroundColourId,canvas);slider.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    slider.setTooltip(hint+". Drag the slider, or double-click the value to type. Double-click the track to reset.");
    addAndMakeVisible(slider);attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,id,slider);
    const juce::String key(id);
    slider.textFromValueFunction=[key](double v){
        if(key=="b1freq")return juce::String(v/1000,2)+" kHz";
        if(key=="b1threshold"||key=="b1range"||key=="output")return juce::String(v,1)+" dB";
        if(key=="b1q")return juce::String(v,2)+" Q";
        if(key=="ratio")return juce::String(v,1)+":1";
        if(key=="attack"||key=="release")return juce::String(v,v<10?2:0)+" ms";
        return juce::String(v,0)+" %";
    };
    slider.valueFromTextFunction=[key](const juce::String& t){const double v=t.getDoubleValue();return key=="b1freq"&&(t.containsIgnoreCase("k")||std::abs(v)<100)?v*1000:v;};
    slider.updateText();
    auto* parameter=p.parameters.getParameter(id);slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
}
void ParameterControl::paint(juce::Graphics& g) {
    label(g,title,{0,0,getWidth(),20},12,ink);
    if(!compact)label(g,hint,{0,119,getWidth(),20},11,muted);
    if(hasKeyboardFocus(true)){g.setColour(border);g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f),4,1);}
}
void ParameterControl::resized(){slider.setBounds(0,compact?21:26,getWidth(),compact?59:80);}

float Spectrum::xFor(float freq) const {
    const float sr=static_cast<float>(p.getSampleRate()>0?p.getSampleRate():48000);
    const float high=std::min(20000.f,sr*.49f);
    return 9+(getWidth()-18)*std::log(std::clamp(freq,500.f,high)/500)/std::log(high/500);
}
float Spectrum::yFor(float db) const {return 8+(getHeight()-26)*std::clamp(-db/84.f,0.f,1.f);}
void Spectrum::update() {
    const unsigned pos=p.samplePosition.load(std::memory_order_acquire);
    if(pos!=lastPosition) {
        lastPosition=pos;data.fill(0);
        for(unsigned i=0;i<1024;++i)data[i]=p.samples[(pos-1024+i)&2047].load(std::memory_order_relaxed)*(.5f-.5f*std::cos(2*static_cast<float>(deess01::pi)*i/1023));
        fft.performFrequencyOnlyForwardTransform(data.data());
        for(int i=0;i<512;++i)values[i]=std::max(deess01::gainToDb(data[i]/256),values[i]-1.8f);
    }else for(auto& v:values)v=std::max(-96.f,v-2.f);
    repaint();
}
void Spectrum::paint(juce::Graphics& g) {
    g.setColour(white);g.fillRoundedRectangle(getLocalBounds().toFloat(),5);
    const float sr=static_cast<float>(p.getSampleRate()>0?p.getSampleRate():48000),high=std::min(20000.f,sr*.49f);
    for(float db:{-24.f,-48.f,-72.f}){g.setColour(soft);g.drawHorizontalLine(static_cast<int>(yFor(db)),9.f,getWidth()-9.f);}
    const float centre=xFor(p.value("b1freq")),width=std::clamp(55.f/p.value("b1q"),12.f,80.f);
    g.setColour(soft);g.fillRect(juce::Rectangle<float>(centre-width/2,8,width,getHeight()-26.f).getIntersection(getLocalBounds().toFloat().reduced(8)));
    juce::Path curve;
    for(int x=9;x<getWidth()-9;++x){const float f=500*std::pow(high/500,static_cast<float>(x-9)/(getWidth()-18));const float bin=f*1024/sr;const int i=std::clamp(static_cast<int>(bin),0,510);const float frac=std::clamp(bin-i,0.f,1.f);const float y=yFor(values[i]+frac*(values[i+1]-values[i]));if(x==9)curve.startNewSubPath(static_cast<float>(x),y);else curve.lineTo(static_cast<float>(x),y);}
    g.setColour(muted);g.strokePath(curve,juce::PathStrokeType(1));g.setColour(ink.withAlpha(.4f));g.drawVerticalLine(static_cast<int>(centre),8.f,getHeight()-18.f);
    for(float f:{1000.f,4000.f,16000.f})if(f<high)label(g,juce::String(static_cast<int>(f/1000))+"k",{static_cast<int>(xFor(f))-13,getHeight()-17,26,14},9,muted,false,juce::Justification::centred);
    g.setColour(border);g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(.5f),5,1);
}

DeessEditor::DeessEditor(DeessProcessor& owner):AudioProcessorEditor(owner),p(owner),spectrum(owner) {
    setLookAndFeel(&skin);
    const std::array<const char*,9> ids{{"b1freq","b1threshold","b1range","b1q","attack","release","ratio","mix","output"}};
    const std::array<const char*,9> names{{"Frequency","Threshold","Range","Focus","Attack","Release","Ratio","Mix","Output"}};
    const std::array<const char*,9> hints{{"Find the sharp consonants","Lower to catch more","Limit the reduction","Narrow the selected frequencies","How quickly reduction starts","How quickly the voice recovers","Compression above the threshold","Blend with the original voice","Adjust the final level"}};
    for(int i=0;i<9;++i){auto control=std::make_unique<ParameterControl>(p,ids[i],names[i],hints[i],i>=3);addAndMakeVisible(*control);control->slider.setLookAndFeel(&skin);controls.push_back(std::move(control));}
    mainTab.setName("De-ess");detailsTab.setName("Details");bypass.setName("Bypass");
    for(auto* b:{&mainTab,&detailsTab,&bypass}){addAndMakeVisible(b);b->setWantsKeyboardFocus(true);}
    mainTab.onClick=[this]{showDetails(false);};detailsTab.onClick=[this]{showDetails(true);};bypass.onClick=[this]{p.setValue("bypass",p.value("bypass")>.5f?0.f:1.f);};
    bypass.setTooltip("Compare the processed voice with the original. All settings are preserved.");
    mode.addItemList({"Split","Wide"},1);mode.setName("Processing");mode.setTooltip("Split reduces only the selected frequency region. Wide turns down the full voice when sibilance is detected.");
    listen.addItemList({"Listen: Processed","Listen: Sibilance","Listen: Removed"},1);listen.setName("Listen");listen.setTooltip("Listen to the final voice, the detector band, or the signal being removed.");
    addAndMakeVisible(mode);addAndMakeVisible(listen);addAndMakeVisible(spectrum);
    modeAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,"mode",mode);
    listenAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,"listen",listen);
    setSize(840,300);showDetails(p.detailsView.load());startTimerHz(30);
}
DeessEditor::~DeessEditor(){stopTimer();setLookAndFeel(nullptr);}
void DeessEditor::showDetails(bool details) {
    p.detailsView=details;mainTab.setToggleState(!details,juce::dontSendNotification);detailsTab.setToggleState(details,juce::dontSendNotification);
    for(size_t i=0;i<controls.size();++i)controls[i]->setVisible(i<3?!details:details);
    repaint();
}
void DeessEditor::resized() {
    mainTab.setBounds(176,20,72,30);detailsTab.setBounds(252,20,72,30);mode.setBounds(348,20,80,30);listen.setBounds(452,20,170,30);bypass.setBounds(696,20,116,30);
    for(int i=0;i<3;++i)controls[i]->setBounds(28+i*182,88,166,143);
    for(int i=3;i<9;++i)controls[i]->setBounds(28+(i-3)%3*182,79+(i-3)/3*91,166,82);
    spectrum.setBounds(606,97,206,110);
}
void DeessEditor::paint(juce::Graphics& g) {
    g.fillAll(canvas);label(g,"DEESS-01",{28,18,124,32},17,ink,true);
    g.setColour(border);g.drawHorizontalLine(66,28.f,812.f);g.drawVerticalLine(580,84.f,244.f);g.drawHorizontalLine(263,28.f,812.f);
    label(g,"INPUT",{606,76,140,16},10,muted);
    label(g,"REDUCTION",{606,219,106,18},10,muted);
    label(g,juce::String(reduction,1)+" dB",{708,213,104,29},22,ink,false,juce::Justification::centredRight);
    g.setColour(border);g.fillRoundedRectangle(606,247,206,3,1.5f);g.setColour(ink);g.fillRoundedRectangle(606,247,206*std::clamp(reduction/15,0.f,1.f),3,1.5f);
    label(g,p.detailsView.load()?"Fine-tune the response. Your main settings stay the same.":"Tame sharp consonants. Keep the voice clear.",{28,273,480,17},11,muted);
    if(clipFrames>0)label(g,"Output clipping - lower Output",{520,273,292,17},11,juce::Colour(0xffb42318),false,juce::Justification::centredRight);
    else {const auto peak=[](float v){return v>1.e-5f?juce::String(deess01::gainToDb(v),1)+" dBFS":juce::String("--");};label(g,"In "+peak(inputLevel)+"   Out "+peak(outputLevel),{520,273,292,17},10,muted,false,juce::Justification::centredRight);}
}
void DeessEditor::timerCallback() {
    if(detailsTab.getToggleState()!=p.detailsView.load())showDetails(p.detailsView.load());
    spectrum.update();reduction=std::max(p.gainReduction[0].exchange(0),reduction-.25f);
    inputLevel=std::max(p.inputPeak.exchange(0),inputLevel*.85f);const float peak=p.outputPeak.exchange(0);outputLevel=std::max(peak,outputLevel*.85f);
    if(peak>1)clipFrames=45;else if(clipFrames>0)--clipFrames;
    const bool disabled=p.value("bypass")>.5f;bypass.setButtonText(disabled?"Bypassed":"Active");bypass.setToggleState(!disabled,juce::dontSendNotification);
    repaint();
}
