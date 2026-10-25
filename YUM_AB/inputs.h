#ifndef INPUTS_H
#define INPUTS_H

#include "globals.h"
#include "player.h"

void checkInputs()
{
  // a held button keeps its direction asked for, a button that was just pressed wins over the others
  if (arduboy.pressed(DOWN_BUTTON))  player.ask(DIR_DOWN);
  if (arduboy.pressed(LEFT_BUTTON))  player.ask(DIR_LEFT);
  if (arduboy.pressed(UP_BUTTON))    player.ask(DIR_UP);
  if (arduboy.pressed(RIGHT_BUTTON)) player.ask(DIR_RIGHT);

  if (arduboy.justPressed(DOWN_BUTTON))  player.ask(DIR_DOWN);
  if (arduboy.justPressed(LEFT_BUTTON))  player.ask(DIR_LEFT);
  if (arduboy.justPressed(UP_BUTTON))    player.ask(DIR_UP);
  if (arduboy.justPressed(RIGHT_BUTTON)) player.ask(DIR_RIGHT);

  if (arduboy.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_GAME_PAUSE;
}

#endif
