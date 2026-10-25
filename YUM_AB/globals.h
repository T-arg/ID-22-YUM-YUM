#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <EEPROM.h>
#include <Arduboy2.h>
#include <Sprites.h>
#include "ATMconfig.h"
#include <ATMlib.h>
#include "bitmaps.h"

// define menu states (on main menu)
#define STATE_MENU_INTRO             0
#define STATE_MENU_MAIN              1
#define STATE_MENU_HELP              2
#define STATE_MENU_PLAY              3
#define STATE_MENU_INFO              4
#define STATE_MENU_SOUNDFX           5

// define game states   (must match the order of mainGameLoop[] in YUM_YUM_AB.ino)
#define STATE_GAME_PREPARE_LEVEL     6
#define STATE_GAME_NEXT_LEVEL        7
#define STATE_GAME_PLAYING           8
#define STATE_GAME_PAUSE             9
#define STATE_GAME_OVER              10

// facing / moving directions, in the order of the yumyum sprite sheet
#define DIR_DOWN                     0     // +Y : screen down (and a little to the left)
#define DIR_LEFT                     1     // -X : screen left
#define DIR_UP                       2     // -Y : screen up   (and a little to the right)
#define DIR_RIGHT                    3     // +X : screen right

Arduboy2Base arduboy;
Sprites sprites;
ATMsynth ATM;

byte gameState = STATE_MENU_INTRO;         // start the game with the TEAM a.r.g. logo
byte menuSelection = STATE_MENU_PLAY;      // PLAY menu item is pre-selected
byte introCounter = 0;
bool newGame = true;                       // set to true to build a new maze when the game starts

#include "music.h"

#endif
