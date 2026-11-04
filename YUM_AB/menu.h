#ifndef MENU_H
#define MENU_H

#include "globals.h"
#include "title.h"
#include "conftext.h"

// the ARDUBOY intro at 45 fps (see the table in arduboy_intro.h), then the small T-arg logo
#define INTRO_STEP        11
#define INTRO_TICKS       6
#define INTRO_SLIDE       15
#define INTRO_SPEED       8
#define INTRO_TAIL        15
#include "arduboy_intro.h"
#include "targ.h"

void stateMenuIntro()
{
  if (introCounter < INTRO_FRAMES) arduboyIntro(introCounter);   // the ARDUBOY letters land on the notes ...
  else
  {
    if (introCounter == INTRO_FRAMES) titleMusic(true);          // the title tune starts when the small T-arg logo appears
    sprites.drawSelfMasked(49, 20, T_arg, 0);                    // ... then the small T-arg logo
  }
  introCounter++;
  if (introCounter > INTRO_FRAMES + 52)
  {
    gameState = STATE_MENU_MAIN;
  }
}

// the title screen: the maze scrolls by, the logo on top, INFO PLAY CONF at the bottom
void stateMenuMain()
{
  titleMusic(true);                                   // (back from the game, or sound just switched on in CONF)
  drawTitleBackground();
  drawLogo();
  drawTitleMenu();
  if (arduboy.justPressed(RIGHT_BUTTON) && (titleChoice < 2)) titleChoice++;
  if (arduboy.justPressed(LEFT_BUTTON) && (titleChoice > 0)) titleChoice--;
  if (arduboy.justPressed(A_BUTTON | B_BUTTON))
  {
    if      (titleChoice == 0) gameState = STATE_MENU_INFO;
    else if (titleChoice == 2) gameState = STATE_MENU_SOUNDFX;
    else                       gameState = STATE_MENU_PLAY;
  }
}

void stateMenuHelp()                                  // (not reachable from the title for now)
{
  if (arduboy.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_MENU_MAIN;
}

void stateMenuInfo()
{
  drawTitleBackground();
  drawLogo();
  arduboy.fillRect(0, MENU_BAR_Y, 128, 8, BLACK);                               // the same black bar as the title menu
  sprites.drawOverwrite(14, MENU_TEXT_Y, text_created, 0);                      // CREATED BY JOERI
  if (arduboy.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_MENU_MAIN;
}

// CONF: LEFT = sound on, RIGHT = sound off, A / B saves it and goes back to the title
void stateMenuSoundfx()
{
  drawTitleBackground();
  drawLogo();
  arduboy.fillRect(0, MENU_BAR_Y, 128, 8, BLACK);
  sprites.drawOverwrite(34, MENU_TEXT_Y, text_sound, 0);                        // SOUND
  if (arduboy.audio.enabled()) sprites.drawOverwrite(73, MENU_TEXT_Y, text_on, 0);   // ON
  else                         sprites.drawOverwrite(73, MENU_TEXT_Y, text_off, 0);  // OFF
  if (arduboy.justPressed(LEFT_BUTTON | UP_BUTTON)) { arduboy.audio.on(); titleMusic(true); }   // sound on: the tune starts again
  if (arduboy.justPressed(RIGHT_BUTTON | DOWN_BUTTON)) { arduboy.audio.off(); titleMusic(false); }   // sound off: the tune stops
  if (arduboy.justPressed(A_BUTTON | B_BUTTON))
  {
    arduboy.audio.saveOnOff();
    gameState = STATE_MENU_MAIN;
  }
}

// PLAY: first the difficulty menu EASY NORMAL HARD (LEFT / RIGHT, A or B starts the game)
void stateMenuPlay()
{
  titleMusic(true);
  drawTitleBackground();
  drawLogo();
  drawLevelMenu();
  if (arduboy.justPressed(RIGHT_BUTTON) && (difficulty < 2)) difficulty++;
  if (arduboy.justPressed(LEFT_BUTTON) && (difficulty > 0)) difficulty--;
  if (arduboy.justPressed(A_BUTTON | B_BUTTON))
  {
    titleMusic(false);                                // the game has no music (only sound effects)
    titleReady = false;                               // the title builds a new maze next time
    newGame = true;
    gameState = STATE_GAME_PREPARE_LEVEL;
  }
}

#endif
