#ifndef TITLE_H
#define TITLE_H

#include "globals.h"
#include "maze.h"
#include "player.h"
#include "enemies.h"
#include "playfield.h"
#include "logo.h"
#include "logosmall.h"
#include "menuwords.h"
#include "levels.h"

/*
  TITLE SCREEN
  ------------------------------------------------------------------------
  The maze scrolls by (the same maze and drawing as the game, without Yum Yum, ghosts and dead zone),
  the logo (logo.h) sits on top of it and at the bottom there is a black bar with the menu:
  INFO  PLAY  CONF. The bar is the bottom 8 px (no line), the menu text fills it; the chosen item is bright, the others dimmed.
*/

#define TITLE_SCROLL    6         // 1/16 px per frame
#define MENU_BAR_Y      52        // the black bar and the words, 4 px up from the bottom of the screen
#define MENU_TEXT_Y     52
#define LOGO_Y          6

bool titleReady = false;          // false: the title builds a new maze next time
byte titleSub = 0;
byte titleChoice = 1;             // 0 = INFO, 1 = PLAY, 2 = CONF

void titleInit()
{
  mazeInit((uint16_t)micros() ^ (uint16_t)(millis() << 3));
  ghostsClear(0);
  player.set();
  player.py = -2000;              // nobody is drawn
  deadY = -30000;                 // no dead zone
  camY = 40;
  titleSub = 0;
  titleReady = true;
}

// the maze that scrolls by
void drawTitleBackground()
{
  if (!titleReady) titleInit();
  titleSub += TITLE_SCROLL;
  camY += titleSub >> 4;
  titleSub &= 15;
  if (camY > 8192 + 400) { camY -= 8192; gen.y -= ROW_SHIFT; }     // endless: shift back (see rebaseWorld in game.h)
  camSX = (MAZE_W * 8) - (camY + 32) - 64;                   // the middle of the maze stays in the middle of the screen
  waveT++;
  drawMaze();
}

// 3 small YUMs twinkle over the logo, like stars: each one shows for 32 of 48 frames, the three are 16 frames apart,
// so there are always 2 on screen. Every time one appears it picks a new place (in its own third of the logo).
#define SPARK_CYCLE     48
#define SPARK_ON        32
#define SPARK_OUT       4       // a small YUM may stick out this many px beyond the logo
byte sparkT = 0;
byte sparkRng = 77;
int16_t sparkX[3] = { 30, 55, 80 };
int16_t sparkY[3] = { 8, 12, 8 };

void drawLogo()
{
  byte w = pgm_read_byte(titleLogo_plus_mask);                                   // YUM, twice, 2 px apart
  int16_t x = (128 - (2 * w + 2)) / 2;
  sprites.drawPlusMask(x, LOGO_Y, titleLogo_plus_mask, 0);
  sprites.drawPlusMask(x + w + 2, LOGO_Y, titleLogo_plus_mask, 0);

  byte sw = pgm_read_byte(titleSmall_plus_mask);
  int16_t span = (2 * w + 2) - sw + 2 * SPARK_OUT;                               // where the left edge of a small YUM can be (up to SPARK_OUT px outside the logo)
  if (++sparkT >= SPARK_CYCLE) sparkT = 0;
  for (byte i = 0; i < 3; i++)
  {
    byte ph = (sparkT + SPARK_CYCLE - 16 * i) % SPARK_CYCLE;
    if (ph == 0)                                                                 // it appears: a new place
    {
      sparkRng = sparkRng * 109 + 89;
      sparkX[i] = x - SPARK_OUT + (span * i) / 3 + (sparkRng >> 3) % (span / 3 + 1);
      sparkRng = sparkRng * 109 + 89;
      sparkY[i] = LOGO_Y - SPARK_OUT + (sparkRng >> 4) % (16 - 8 + 2 * SPARK_OUT + 1);
    }
    if (ph < SPARK_ON)
      sprites.drawPlusMask(sparkX[i], sparkY[i], titleSmall_plus_mask, 0);
  }
}

// a black panel with a white edge (for the INFO and CONF screens)
void drawPanel(int16_t x, int16_t y, int16_t w, int16_t h)
{
  arduboy.fillRect(x, y, w, h, BLACK);
  arduboy.drawRect(x, y, w, h, WHITE);
}

// a little ball bounces above the chosen word (like the egg in ID-39)
const uint8_t PROGMEM menuBounce[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0 };
byte menuAnim = 0;

#define MENU_X0         17        // the three words (30 wide) are centred, 2 px apart

void drawTitleMenu()
{
  arduboy.fillRect(0, MENU_BAR_Y, 128, 8, BLACK);     // the black mask
  for (byte i = 0; i < 3; i++)
  {
    int16_t x = MENU_X0 + 32 * i;
    sprites.drawOverwrite(x, MENU_TEXT_Y, menuWords, i);            // INFO, PLAY, CONF
  }
  // the big ball (10 x 10 in its 16 x 16 sprite), centred on the spot where the little ball was
  sprites.drawPlusMask(MENU_X0 + 32 * titleChoice + 10, 44 - pgm_read_byte(&menuBounce[(menuAnim >> 1) & 15]), bigBall_plus_mask, 0);
  menuAnim++;
}

// the difficulty menu EASY NORMAL HARD: the same black bar, the same big bouncing ball above the chosen word

#define LEVEL_X0        12        // EASY (27 wide), NORMAL (39), HARD (27), 5 px apart: centred
#define LEVEL_GAP       5

void drawLevelMenu()
{
  static const int8_t levelX[3] = { LEVEL_X0, LEVEL_X0 + 27 + LEVEL_GAP, LEVEL_X0 + 27 + LEVEL_GAP + 39 + LEVEL_GAP };
  static const int8_t levelW[3] = { 27, 39, 27 };
  arduboy.fillRect(0, MENU_BAR_Y, 128, 8, BLACK);
  sprites.drawOverwrite(levelX[0], MENU_TEXT_Y, level_easy, 0);
  sprites.drawOverwrite(levelX[1], MENU_TEXT_Y, level_normal, 0);
  sprites.drawOverwrite(levelX[2], MENU_TEXT_Y, level_hard, 0);
  sprites.drawPlusMask(levelX[difficulty] + levelW[difficulty] / 2 - 5, 44 - pgm_read_byte(&menuBounce[(menuAnim >> 1) & 15]), bigBall_plus_mask, 0);
  menuAnim++;
}

#endif
