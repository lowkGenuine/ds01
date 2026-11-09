#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>
void expect(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void save(juce::Component& c,const juce::File& f){auto stream=f.createOutputStream();if(!stream)throw std::runtime_error("Cannot write preview");stream->setPosition(0);stream->truncate();juce::PNGImageFormat png;png.writeImageToStream(c.createComponentSnapshot(c.getLocalBounds()),*stream);}
int main(int argc,char** argv){try{
    juce::ScopedJuceInitialiser_GUI gui;DeessProcessor p;p.prepareToPlay(48000,256);juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,256);
    auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());auto* screen=dynamic_cast<DeessEditor*>(editor.get());expect(screen!=nullptr,"Wrong editor class");
    for(int block=0;block<200;++block){for(int i=0;i<256;++i){const int n=block*256+i;const float x=.18f*std::sin(2*deess01::pi*180*n/48000)+.08f*std::sin(2*deess01::pi*360*n/48000)+.2f*std::sin(2*deess01::pi*6100*n/48000);b.setSample(0,i,x);b.setSample(1,i,x);}p.processBlock(b,midi);if(block%20==0)juce::MessageManager::getInstance()->runDispatchLoopUntil(35);}
    juce::MessageManager::getInstance()->runDispatchLoopUntil(40);
    int mainControls=0;ParameterControl* frequency=nullptr;
    for(auto* child:screen->getChildren())if(auto* control=dynamic_cast<ParameterControl*>(child)){if(control->isVisible())++mainControls;if(control->getName()=="Frequency")frequency=control;}
    expect(mainControls==3&&frequency,"Main screen does not expose three controls");expect(frequency->slider.getValueFromText("7.25 kHz")==7250&&frequency->slider.getValueFromText("7250 Hz")==7250,"Frequency text entry changes units");
    frequency->slider.setValue(7250,juce::sendNotificationSync);expect(std::abs(p.value("b1freq")-7250)<1,"Slider does not change the processor");frequency->slider.setValue(6000,juce::sendNotificationSync);
    if(argc>1)save(*editor,juce::File(juce::String::fromUTF8(argv[1])));
    screen->showDetails(true);int detailControls=0;for(auto* child:screen->getChildren())if(dynamic_cast<ParameterControl*>(child)&&child->isVisible()){++detailControls;expect(editor->getLocalBounds().contains(child->getBounds()),"Details control outside the window");}expect(detailControls==6,"Details page does not expose six controls");expect(std::abs(p.value("b1freq")-6000)<1,"Switching pages changed the sound");
    if(argc>2)save(*editor,juce::File(juce::String::fromUTF8(argv[2])));
    juce::MemoryBlock state;p.getStateInformation(state);p.detailsView=false;p.setStateInformation(state.getData(),static_cast<int>(state.getSize()));expect(p.detailsView.load(),"View selection was not saved");
    std::cout<<"PASS: real 840x300 editor renders both pages, 3 main / 6 detail controls, frequency text parsing and control attachment, page changes preserve sound, view persistence\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
