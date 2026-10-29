#ifndef MENU_H
#define MENU_H

#include "globals.h"
#include "title.h"

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
  drawPanel(10, 14, 108, 36);
  arduboy.drawBitmap(16, 20, info, 96, 24, WHITE);
  if (arduboy.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_MENU_MAIN;
}

// CONF: LEFT = sound on, RIGHT = sound off, A / B saves it and goes back to the title
void stateMenuSoundfx()
{
  drawTitleBackground();
  drawPanel(24, 20, 80, 24);
  sprites.drawSelfMasked(34, 28, menuText, 0);                                  // SFX
  sprites.drawSelfMasked(74, 28, menuText, arduboy.audio.enabled() ? 2 : 1);    // ON / OFF
  if (arduboy.justPressed(LEFT_BUTTON | UP_BUTTON)) arduboy.audio.on();
  if (arduboy.justPressed(RIGHT_BUTTON | DOWN_BUTTON)) { arduboy.audio.off(); titleMusic(false); }   // sound off: the tune stops
  if (arduboy.justPressed(A_BUTTON | B_BUTTON))
  {
    arduboy.audio.saveOnOff();
    gameState = STATE_MENU_MAIN;
  }
}

void stateMenuPlay()
{
  titleMusic(false);                                  // the game has no music (only sound effects)
  titleReady = false;                                 // the title builds a new maze next time
  newGame = true;
  gameState = STATE_GAME_PREPARE_LEVEL;
}

#endif
