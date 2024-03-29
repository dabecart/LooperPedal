#include "AudioPlayer.h"
#include "UI/MenuManager.h"
#include "UI/GUI/Taskbar.h"
#include "UI/GUI/Widgets/DemoWidget.h"
#include "UI/GUI/Widgets/AudioPlayerWidget.h"
#include "UI/Display.h"
#include "UI/GUI/Menu.h"

void launchInitialWidgets();

void setup() {
  Serial.begin(115200);
  
  Utilities::debug("\n*********************************************\n");
  Utilities::debug("    LOOPER PEDAL v0.1, by DanielUnleashed\n");
  Utilities::debug("*********************************************\n");
  delay(50);

  MenuManager::init();
  AudioPlayer::SDBoot();

  AudioPlayer::addRECAudioFile(0);
  AudioPlayer::addSDAudioFile("/tereza.wav"); 

  launchInitialWidgets();

  #ifndef LAUNCH_SPLASHSCREEN_AT_BOOT_UP
    AudioPlayer::begin();
  #endif
}

void loop() {}

void launchInitialWidgets(){
#ifndef USE_DISPLAY 
    return;
#else

  //DemoWidget* demo = new DemoWidget(0,2,2,2);
  AudioPlayerWidget* aud = new AudioPlayerWidget(0,0,6,2);

  static Display mainDisplay("Main");
  mainDisplay.addTaskbar();
  //mainDisplay.addItem(demo);
  mainDisplay.addItem(aud);
  //mainDisplay.addItem(new Menu("Main Menu"));

  MenuManager::addDisplay(mainDisplay);
  MenuManager::launch();
#endif
}