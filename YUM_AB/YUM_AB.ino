/*
  YUM YUM: https://t-arg.github.io/ID-22-YUM-YUM/

  Arduboy version 0.2:  https://github.com/T-arg/ID-22-YUM-YUM

  MADE by TEAM a.r.g. : https://t-arg.github.io/

  2016 - JO3RI - STG

  License: MIT : https://opensource.org/licenses/MIT

*/

//determine the game
#define GAME_ID 22

#include "globals.h"
#include "menu.h"
#include "game.h"
#include "inputs.h"

// Arduboy2 keeps the sound on/off flag at this EEPROM address (not every library version exposes the name)
#ifndef EEPROM_AUDIO_ON_OFF
#define EEPROM_AUDIO_ON_OFF 2
#endif

typedef void (*FunctionPointer) ();
const FunctionPointer PROGMEM mainGameLoop[] = {
  stateMenuIntro,
  stateMenuMain,
  stateMenuHelp,
  stateMenuPlay,
  stateMenuInfo,
  stateMenuSoundfx,
  stateGamePrepareLevel,
  stateGameNextLevel,
  stateGamePlaying,
  stateGamePause,
  stateGameOver,
};


void setup() {
  arduboy.boot();
  if (EEPROM.read(EEPROM_AUDIO_ON_OFF)) arduboy.audio.on();     // check EEPROM if sound is OFF or ON
  else arduboy.audio.off();
  arduboy.setFrameRate(45);                                     // 45 fps like ID-39: the maze draws a lot of tiles
}


void loop() {
  if (!(arduboy.nextFrame())) return;
  arduboy.pollButtons();
  sfxUpdate();
  arduboy.clear();
  ((FunctionPointer) pgm_read_word(&mainGameLoop[gameState]))();
  arduboy.display();
}
