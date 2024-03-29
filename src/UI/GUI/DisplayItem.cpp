#include "DisplayItem.h"
#include "UI/MenuManager.h"

uint16_t DisplayItem::width, DisplayItem::height;

DisplayItem::DisplayItem(String n){
    itemName = n;
}

void DisplayItem::attachEvents(){
    for(ButtonEvent ev : buttonEvents){
        if(ev.name.equals("")) DebounceButton::addInterrupt(ev.pin, ev.func, ev.functionalEvent);
        else DebounceButton::addInterrupt(ev.pin, ev.name, ev.func, ev.functionalEvent);
    }
    for(RotaryEncoderEvent ev : rotaryEvents) RotaryEncoder::addInterrupt(ev.pin, ev.func);
    for(ButtonEvent ev : rotaryButtonEvents) DebounceButton::addRotaryInterrupt(ev.pin, ev.func, ev.functionalEvent);
}

void DisplayItem::addButtonEvent(uint8_t buttonIndex, std::function<void()> func){
    ButtonEvent ev = {buttonIndex, func, "", 0};
    buttonEvents.push_back(ev);
}

void DisplayItem::addButtonEvent(uint8_t buttonIndex, std::function<void()> func, String name){
    ButtonEvent ev = {buttonIndex, func, name, 0};
    buttonEvents.push_back(ev);
}

void DisplayItem::addButtonEvent(uint8_t buttonIndex, std::function<void()> func, String name, uint8_t mode){
    ButtonEvent ev = {buttonIndex, func, name, mode};
    buttonEvents.push_back(ev);
}

void DisplayItem::addRotaryEvent(uint8_t rotatoryIndex, std::function<void(bool incr)> func){
    RotaryEncoderEvent ev = {rotatoryIndex, func};
    rotaryEvents.push_back(ev);
}

void DisplayItem::addRotaryButtonEvent(uint8_t buttonIndex, std::function<void(void)> func){
    ButtonEvent ev = {buttonIndex, func, "", 0};
    rotaryButtonEvents.push_back(ev);
}

void DisplayItem::addRotaryButtonEvent(uint8_t buttonIndex, std::function<void(void)> func, uint8_t mode){
    ButtonEvent ev = {buttonIndex, func, "", mode};
    rotaryButtonEvents.push_back(ev);
}

void DisplayItem::startDisplayItems(uint16_t w, uint16_t h){
    width = w;
    height = h;
}

void DisplayItem::startAnimation(){
    startAnimationTime = micros();
    redraw();
}

void DisplayItem::render(TFT_eSprite &c){
    canvas = &c;
    draw();
}

void DisplayItem::redraw(){
    needsUpdate = true;
    if(xPortInIsrContext()) MenuManager::wakeUpDrawTask();
    else MenuManager::wakeUpDrawTaskFromISR();
}

void DisplayItem::animationRedraw(){
    redraw();
}

void DisplayItem::endAnimation(){
    needsUpdate = false;
}

bool DisplayItem::needsToRedraw(){
    return needsUpdate;
}

void DisplayItem::forceRedraw(){
    needsUpdate = true;
}

uint32_t DisplayItem::getTickTime(){
    return (micros() - startAnimationTime)/10000.0;
}