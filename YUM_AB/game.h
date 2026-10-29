#ifndef GAME_H
#define GAME_H

#include "globals.h"
#include "maze.h"
#include "player.h"
#include "enemies.h"
#include "inputs.h"
#include "playfield.h"
#include "title.h"
#include "gameover.h"

/*
  THE DEAD ZONE
  The top of the maze is eaten by black, slowly creeping down (like the dead zone in ID-39, but this
  time you run DOWN the maze). It only starts moving after the first step. Touch it and it is over.
*/
#define DEAD_START      40        // px the dead zone starts above Yum Yum
#define DEAD_LAG        96        // it is never more than this many px (12 cell rows) behind the furthest row reached
#define DEAD_SPEED      3         // 1/16 px per frame at the start (about 1 cell row per second)
#define DEAD_SPEED_MAX  9         // ... and the speed it gets to (3 times faster)
#define DEAD_RAMP       40        // cell rows per extra 1/16 px per frame
#define HURT_TIME       90        // frames Yum Yum blinks (and can not be hit) after a hit

byte deadSub = 0;                 // 1/16 px
bool lostToDeadZone = false;

// the seed of the maze: a new one for every game, B on the game over screen plays the same maze again.
// FIXED_SEED: any number 1..65535 gives that maze every time (0 = a new maze every game)
#define FIXED_SEED      0
bool sameMaze = false;

void resetGame()
{
  uint16_t seed = FIXED_SEED;
  if (sameMaze) seed = mazeSeed;
  else if (!seed) seed = (uint16_t)micros() ^ (uint16_t)(millis() << 3);          // (the time between button presses is random enough)
  sameMaze = false;
  rngState = (uint16_t)micros() | 1;                                              // the ghosts: not seeded
  for (uint8_t i = 0; i < 8; i++) rnd();
  mazeInit(seed);
  player.set();
  ghostsClear(120);
  blastR = 0;
  ensureRows(24);
  eatDot(START_CX, START_CY);                                      // nothing to eat on the start cell
  camY   = (player.py >> 1) - PLAYER_SY;
  camSX  = player.px - (player.py >> 1) - 64;
  deadY  = (player.py >> 1) - DEAD_START;
  deadSub = 0;
  freezeTime = 0;
  lostToDeadZone = false;
}

// the world is made of 16 bit numbers: far down the maze everything is moved back a bit (the maze
// lives in a ring buffer, so it does not notice)
__attribute__((noinline)) static void rebaseWorld()
{
  if (player.py < 24000) return;
  const int16_t unitsShift = ROW_SHIFT * 16;
  const int16_t pxShift = ROW_SHIFT * 8;
  player.py -= unitsShift;
  player.bestRow -= ROW_SHIFT;
  gen.y -= ROW_SHIFT;
  for (byte i = 0; i < MAX_GHOSTS; i++) ghosts[i].py -= unitsShift;
  blastY -= unitsShift;
  camY -= pxShift;
  camSX += ROW_SHIFT * 8;                               // (the screen x of a cell depends on its row: -4 px per row)
  deadY -= pxShift;
}

// camera: glides towards Yum Yum, always moving at least 1 px
static int16_t camStep(int16_t d)
{
  if (d > 0) return (d + 3) >> 2;
  return -((-d + 3) >> 2);
}

__attribute__((noinline)) static void updateCamera()
{
  camY  += camStep(((player.py >> 1) - PLAYER_SY) - camY);
  camSX += camStep((player.px - (player.py >> 1) - 64) - camSX);
}

__attribute__((noinline)) static void updateDeadZone()
{
  if (!player.started) return;
  byte spd = DEAD_SPEED + player.bestRow / DEAD_RAMP;
  if (spd > DEAD_SPEED_MAX) spd = DEAD_SPEED_MAX;
  deadSub += spd;
  deadY += deadSub >> 4;
  deadSub &= 15;
  int16_t bestY = player.bestRow * 8;
  if (deadY < bestY - DEAD_LAG) deadY = bestY - DEAD_LAG;
  if (((player.py >> 1) - 2) < deadY) { lostToDeadZone = true; player.lives = 0; freezeTime = HIT_FREEZE; sfx(SFX_DIE, 1, 860); }
}

void stateGamePrepareLevel()
{
  newGame = false;
  resetGame();
  gameState = STATE_GAME_NEXT_LEVEL;
}

void stateGameNextLevel()
{
  gameState = STATE_GAME_PLAYING;
}

void stateGamePlaying()
{
  if (newGame) { resetGame(); newGame = false; }

  checkInputs();
  if (freezeTime)                                       // a hit: everything stands still for a moment
  {
    freezeTime--;
    if (!freezeTime)
    {
      if (player.lives == 0) gameState = STATE_GAME_OVER;
      else
      {
        ghostsClear(150);                               // the ghosts go away for a while
        player.hurt = HURT_TIME;
      }
    }
  }
  else
  {
    player.update();
    ghostsUpdate();
    blastUpdate();
    dotsTick();
    updateDeadZone();
    if (!freezeTime && ghostsHit())
    {
      player.lives--;
      freezeTime = HIT_FREEZE;
      sfx(SFX_DIE, 1, 860);
    }
  }
  waveT++;
  rebaseWorld();
  updateCamera();
  drawPlayField();
}

void stateGamePause()
{
  drawPlayField();                                      // show the frozen scene
  arduboy.fillRect(34, 24, 60, 16, WHITE);
  arduboy.drawRect(35, 25, 58, 14, BLACK);
  drawTextCentered(29, "PAUSE", 2, BLACK);
  if (arduboy.justPressed(A_BUTTON | B_BUTTON)) gameState = STATE_GAME_PLAYING;
}

void stateGameOver()
{
  drawPlayField();                                      // show where it went wrong
  sprites.drawPlusMask(49, 24, gameOver_plus_mask, 0);  // the "Game Over" words of ID-39, 30 x 16, in the middle of the screen
  if (arduboy.justPressed(A_BUTTON))                    // A: back to the title screen (a new maze)
  {
    newGame = true;
    titleReady = false;
    gameState = STATE_MENU_MAIN;
  }
  else if (arduboy.justPressed(B_BUTTON))               // B: play again, in the same maze
  {
    newGame = true;
    sameMaze = true;
    gameState = STATE_GAME_PLAYING;
  }
}

#endif
