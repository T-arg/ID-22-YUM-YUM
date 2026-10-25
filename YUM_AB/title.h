#ifndef TITLE_H
#define TITLE_H

#include "globals.h"
#include "maze.h"
#include "player.h"
#include "enemies.h"
#include "playfield.h"
#include "logo.h"

/*
  TITLE SCREEN
  ------------------------------------------------------------------------
  The maze scrolls by (the same maze and drawing as the game, without Yum Yum, ghosts and dead zone),
  the logo (logo.h) sits on top of it and at the bottom there is a black bar with the menu:
  INFO  PLAY  CONF. The bar is the bottom 8 px (no line), the menu text fills it; the chosen item is bright, the others dimmed.
*/

#define TITLE_SCROLL    6         // 1/16 px per frame
#define MENU_BAR_Y      56
#define MENU_TEXT_Y     56        // the menu text is 8 px high: it fills the bar
#define LOGO_Y          6

bool titleReady = false;          // false: the title builds a new maze next time
byte titleSub = 0;
byte titleChoice = 1;             // 0 = INFO, 1 = PLAY, 2 = CONF

void titleInit()
{
  mazeInit();
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
  camSX = (MAZE_W * 8) - ((camY + 32) >> 1) - 64;                  // the middle of the maze stays in the middle of the screen
  waveT++;
  drawMaze();
}

void drawLogo()
{
  sprites.drawPlusMask((128 - pgm_read_byte(titleLogo_plus_mask)) / 2, LOGO_Y, titleLogo_plus_mask, 0);
}

// a black panel with a white edge (for the INFO and CONF screens)
void drawPanel(int16_t x, int16_t y, int16_t w, int16_t h)
{
  arduboy.fillRect(x, y, w, h, BLACK);
  arduboy.drawRect(x, y, w, h, WHITE);
}

void drawTitleMenu()
{
  arduboy.fillRect(0, MENU_BAR_Y, 128, 64 - MENU_BAR_Y, BLACK);     // the black mask
  static const byte frame[3] = { 2, 1, 3 };                         // INFO, PLAY, CONF in menuText
  for (byte i = 0; i < 3; i++)
  {
    int16_t x = 14 + 40 * i;
    bool on = (i == titleChoice);
    int16_t y = MENU_TEXT_Y;
    sprites.drawSelfMasked(x, y, menuText, frame[i]);
    if (!on)                                                        // the others are dimmed
    {
      for (byte yy = 0; yy < 8; yy++) for (byte xx = 0; xx < 20; xx++)
        if ((xx + yy) & 1) arduboy.drawPixel(x + xx, y + yy, BLACK);
    }
  }
}

#endif
