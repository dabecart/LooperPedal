#include "Taskbar.h"
#include "UI/GUI/Widgets/Widget.h"

Taskbar::Taskbar() : DisplayItem("Taskbar"){}

void Taskbar::draw(){
    canvas -> fillRect(0, height-TASKBAR_HEIGHT, width, TASKBAR_HEIGHT, TFT_BLACK);
    //canvas -> fillRoundRect(0,height-TASKBAR_HEIGHT, width,TASKBAR_HEIGHT, 2, 0x190A);
    for(uint8_t i = 0; i < 4; i++){
        if(!buttons[i].isEnabled) continue;

        //This doesn't work with sprites, dunno why...
        //drawFilledRect(i*25,0, 25,100, 0xEFBE, 0xB63C, true);
        canvas -> fillRoundRect(width*i/4, height-TASKBAR_HEIGHT, width/4,TASKBAR_HEIGHT, 2, 0x2AAF);
        canvas -> setTextColor(TFT_WHITE);
        canvas -> setTextDatum(MC_DATUM); //Medium center datum
        canvas -> drawString(buttons[i].tagName, width*(2*i+1)/8, height-(TASKBAR_HEIGHT/2), 2);
        canvas -> drawFastVLine(width*i/4, height-(TASKBAR_HEIGHT*3/4), TASKBAR_HEIGHT, TFT_WHITE);
        canvas -> drawFastVLine(width*(i+1)/4-1, height-(TASKBAR_HEIGHT*3/4), TASKBAR_HEIGHT, TFT_WHITE);
    }

    needsUpdate = false;
}

bool Taskbar::addButton(uint8_t index, String tagName){
    if(index >= TOTAL_BUTTONS) Utilities::error("Button index %d out of bounds (max. %d)\n", index, TOTAL_BUTTONS);
    if(buttons[index].isEnabled) return false;
    if(tagName.equals("")) return false;
    
    buttons[index].isEnabled = true;
    if(tagName.length() > 5){
        tagName = tagName.substring(0,5);
    }
    buttons[index].tagName = tagName;
    return true;
}

bool Taskbar::removeButton(uint8_t index){
    buttons[index].isEnabled = false;
    return true;
}

void Taskbar::saveAndRemoveButtons(){
    for(uint8_t i = 0; i < 4; i++){
        previousButtons[i].isEnabled = buttons[i].isEnabled;
        previousButtons[i].tagName = buttons[i].tagName;
        removeButton(i);
    }
}

void Taskbar::undoRemoveButtons(){
    for(uint8_t i = 0; i < 4; i++){
        removeButton(i);
        if(previousButtons[i].isEnabled) addButton(i, previousButtons[i].tagName);
    }
}