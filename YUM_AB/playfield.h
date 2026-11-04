#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include "globals.h"
#include "maze.h"
#include "player.h"
#include "enemies.h"
#include "balls.h"
#include "scared.h"
#include "ghost.h"
#include "ghostparts.h"
#include "eyes.h"
#include "spring.h"
#include "bomb.h"
#include "dots.h"
#include "digits.h"
#include "wallpack.h"
#include "fruit.h"

/*
  DRAWING
  ------------------------------------------------------------------------
  The X axis is horizontal, the Y axis runs down the screen and a little to the left (4 px per cell row):

      screen x = X - Y / 2 - camSX (8 px per cell row)              X, Y : world position in pixels
      screen y = Y - camY                       (a cell is 16 px wide and 8 px high: X = 16 cx, Y = 8 cy)

  Painter's algorithm, from the top row of cells to the bottom one: the tiles of a row, then the ghosts
  and Yum Yum that stand in that row. So a wall in the row below covers the feet of whoever walks in
  front of it. The dead zone is painted over everything at the end.

  A wall tile is 23 x 16 (the bottom left pixel is the bottom left corner of the cell, so the tile is drawn 8 px above sy; the upper 8 rows overhang the row above). OLD: 21 x 16: the lower 8 rows are the floor of its own cell, the upper 8 rows are the top
  of the wall, sticking out over the cell above. The floor of the cell is its parallelogram with its
  top left corner at (16 cx - 4 cy, 8 cy); the tile starts 4 px to the left and 8 px above that.
*/

// (FLOOR_GRID is defined in globals.h: it decides if the floor tile is in the flash at all)
#define PLAYER_SY       22      // screen y of the centre of Yum Yum's cell
#define SPRITE_LIFT     3       // px the ball floats above the floor
#define DOT_OX          9       // top left of the 6x8 ball sprite inside the tile (the ball is centred on the floor)
#define DOT_OY          -1      // (the ball fills rows 0-5 of its sprite: centre = 3,3 -> floor centre 10,12)

byte waveT = 0;                 // animation clock of the bouncing dots (counts game frames)

// height (px) of a dot in the wave: one sine period in 16 steps
const uint8_t PROGMEM dotBounce[16] = { 2, 2, 3, 3, 3, 3, 3, 2, 2, 1, 0, 0, 0, 0, 0, 1 };

// which wall tile for which neighbours: bit 0 = wall above, 1 = below, 2 = left, 3 = right
// (the order of the pipe tiles: 0 right, 1 left+right, 2 left, 3 down, 4 up+down, 5 up, 6 up+right, 7 down+right,
//  8 up+left, 9 down+left, 10 all four, 11 up+left+right, 12 up+down+right, 13 down+left+right, 14 up+down+left)
const uint8_t PROGMEM wallTileOf[16] = { 4, 5, 3, 4, 2, 8, 9, 14, 0, 6, 7, 12, 1, 11, 13, 10 };

static inline uint8_t tileAt(int16_t x, int16_t y)
{
  uint8_t m = (wallBit(x, y - 1) ? 1 : 0) | (wallBit(x, y + 1) ? 2 : 0) | (wallBit(x - 1, y) ? 4 : 0) | (wallBit(x + 1, y) ? 8 : 0);
  return pgm_read_byte(&wallTileOf[m]);
}

// screen position of the centre of the floor at unit position (px, py)
static inline int16_t unitScreenX(int16_t px, int16_t py) { return px - (py >> 1) - camSX; }
static inline int16_t unitScreenY(int16_t py)             { return (py >> 1) - camY; }

// a wall tile (23 x 16, image + mask like Sprites::drawPlusMask): the tile is unpacked from wallPack[] (run length coding, see
// tools/packwalls.py) into 92 bytes of RAM: 46 image bytes (page 0, page 1) then 46 mask bytes, and blitted to the screen buffer.
static void drawWallTile(int16_t x, int16_t y, byte t)
{
  byte tile[92];
  const byte *p = wallPack + pgm_read_word(&wallOff[t]);
  for (byte n = 0; n < 92;)
  {
    byte c = pgm_read_byte(p++);
    if (c & 128) { c -= 126; byte v = pgm_read_byte(p++); while (c--) tile[n++] = v; }
    else         { c++;                                     while (c--) tile[n++] = pgm_read_byte(p++); }
  }
  byte *scr = arduboy.getBuffer();
  int8_t page = y >> 3;
  byte sh = y & 7;
  for (byte c = 0; c < 23; c++)
  {
    int16_t sx = x + c;
    if (sx < 0 || sx > 127) continue;
    for (byte q = 0; q < 4; q++)                                   // 2 source pages, each hits 2 screen pages
    {
      int8_t tp = page + (q >> 1) + (q & 1);
      if (tp < 0 || tp > 7) continue;
      byte i = tile[(q >> 1) * 23 + c], m = tile[46 + (q >> 1) * 23 + c];
      uint16_t ii = (uint16_t)i << sh, mm = (uint16_t)m << sh;
      if (q & 1) { ii >>= 8; mm >>= 8; }
      byte *d = scr + tp * 128 + sx;
      *d = (*d & ~(byte)mm) | (byte)ii;
    }
  }
}

static void drawYumYum()
{
  int16_t cx = unitScreenX(player.px, player.py) - 9, cy = unitScreenY(player.py) - 9 - SPRITE_LIFT - player.lift();
  if (freezeTime)                                                                // a hit: Yum Yum shrinks away
  {
    byte t = HIT_FREEZE - freezeTime;
    if (t < DEATH_SPIN)                                                          // turning round, twice, with the normal sprite (open mouth)
    {
      drawStrips(cx, cy, FR_YUM + ((player.dir + t / 6) & 3) * 4 + 3);
      return;
    }
    byte f = (t - DEATH_SPIN) / 6;
    drawStrips(cx, cy, FR_DEATH + ((f > 5) ? 5 : f));
    return;
  }
  if (player.lives == 0) return;                                                 // game over: gone
  if (player.hurt && (player.hurt & 4)) return;                                  // blinks after a hit
  drawStrips(cx, cy, FR_YUM + player.frame());
}

#define GHOST_EYE_Y     4       // the eyes sit this many px below the top of the head

static void drawGhostsInRow(int16_t y)
{
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    const Ghost &g = ghosts[i];
    if (!g.active || (g.py >> 4) != y) continue;
    int16_t gx = unitScreenX(g.px, g.py) - 9, gy = unitScreenY(g.py) - 9 - SPRITE_LIFT;
    byte feet = (g.anim >> 3) & 3;                                               // the 4 feet frames, the eyes change every 2
    gy -= 6;                                                                     // 18 x 24: the same feet as the 18 x 18 sprites, 6 px higher
    if (g.eyes)                                                                  // eaten: just the eyes
      sprites.drawPlusMask(gx, gy + GHOST_EYE_Y, gpEyes, g.dir * 2 + (feet >> 1));
    else
    {
      bool scared = scaredTime && !((scaredTime < 120) ? (scaredTime & 4) : (scaredTime & 16));   // frightened: blinks between blue and normal (faster the last 2 seconds)
      sprites.drawPlusMask(gx, gy, gpHead, scared);
      sprites.drawPlusMask(gx, gy + GHOST_EYE_Y, gpEyes, g.dir * 2 + (feet >> 1));            // the frightened ghost has the same eyes
      sprites.drawPlusMask(gx, gy + 16, gpBase, scared ? 4 + feet : feet);
    }
  }
}

// the dots, the pellets and the pick-ups of one row of cells (painted after the tiles of that row)
static void drawRowItems(int16_t y)
{
  int16_t sy  = 8 * y - camY;
  int16_t sx0 = -8 * y - 8 - camSX;
  int16_t xLo = (-23 - sx0) >> 4, xHi = (127 - sx0) >> 4;
  if (xLo < 0) xLo = 0;
  if (xHi > MAZE_W - 1) xHi = MAZE_W - 1;
  for (int16_t x = xLo; x <= xHi; x++)
  {
    int16_t sx = sx0 + 16 * x;
    if (sx <= -23 || sx >= 128 || wallBit(x, y)) continue;
    byte fk = fruitAt(x, y);
    if (fk)                                                                      // a fruit: floats a little above the floor
    {
      byte fw = pgm_read_byte(fruit_plus_mask), fh = pgm_read_byte(fruit_plus_mask + 1);
      int16_t hop = pgm_read_byte(&dotBounce[((waveT >> 1) + y) & 15]) >> 1;
      sprites.drawPlusMask(sx + 12 - fw / 2, sy + 3 - fh / 2 - hop, fruit_plus_mask, fk - 1);
      continue;
    }
    if (pelletAt(x, y) && powerTime) continue;                                   // no power-ups on the playfield while the timer runs
    if (pelletAt(x, y) && (x == 7 || x == 9))                                    // the spring (power-up JUMP) or the bomb: it hops a little
    {
      int16_t hop = pgm_read_byte(&dotBounce[((waveT >> 1) + y) & 15]) >> 1;
      byte f = (waveT >> 3) % 3;                                                // the 3 frames of the animation
      const unsigned char *spr = bombAt(x, y) ? bomb_plus_mask : dotsAt(x, y) ? dots_plus_mask : spring_plus_mask;
      sprites.drawPlusMask(sx + 4, sy - 5 - hop, spr, f);                       // (all three are 16 x 16)
    }
    else if (pelletAt(x, y))                                                     // power pellet: the big ball, it only blinks
    {
      if (waveT & 8) continue;
      sprites.drawPlusMask(sx + 7, sy - 1, bigBall_plus_mask, 0);                // (stays on the floor, no bounce)
    }
    else if (dotAt(x, y))                                                        // white dots, bouncing in a wave that runs across the maze
    {
      byte lift = pgm_read_byte(&dotBounce[((waveT >> 1) + x * 2 + y) & 15]);
      sprites.drawPlusMask(sx + DOT_OX, sy + DOT_OY - lift, littleBalls_plus_mask, 0);
    }
  }
}

/*
  Paint order: for every row of cells: its tiles, the ghosts of the row, and (a row later) Yum Yum, who is bigger than
  his cell. He is painted after the tiles of his own row and before those of the row below, so the pipes of the row below
  are in front of him. The dots of his own row and of the row above are painted after him, so they are in front of him.
*/
void drawMaze()
{
  int16_t yFirst = (camY >> 3) - 1;
  int16_t yLast  = (camY + 72) >> 3;
  if (yFirst < deadRow() - 1) yFirst = deadRow() - 1;                            // nothing to see in the dead zone
  if (yFirst < 0) yFirst = 0;
  ensureRows(yLast + 2);

  int16_t pRow = player.py >> 4;
  for (int16_t y = yFirst; y <= yLast; y++)
  {
    int16_t sy  = 8 * y - camY;
    int16_t sx0 = -8 * y - 8 - camSX;
    int16_t xLo = (-23 - sx0) >> 4, xHi = (127 - sx0) >> 4;                      // only the columns that are on the screen
    if (xLo < 0) xLo = 0;
    if (xHi > MAZE_W - 1) xHi = MAZE_W - 1;
    for (int16_t x = xLo; x <= xHi; x++)
    {
      int16_t sx = sx0 + 16 * x;
      if (sx <= -23 || sx >= 128) continue;
      if (wallBit(x, y)) drawWallTile(sx, sy - 8, tileAt(x, y));
#if FLOOR_GRID
      else sprites.drawPlusMask(sx, sy, floorTile_plus_mask, 0);
#endif
    }
    if (y != pRow - 1 && y != pRow) drawRowItems(y);
    drawGhostsInRow(y);
    if (pRow == y)                                                               // Yum Yum, then the dots of his row and the row above in front of him
    {
      drawYumYum();
      if (y - 1 >= yFirst) drawRowItems(y - 1);
      drawRowItems(y);
    }
  }

  int16_t d = deadY - camY;                                                      // the dead zone: black, with a ragged edge
  if (d > 0)
  {
    arduboy.fillRect(0, 0, 128, d, BLACK);
    for (byte x = 0; x < 128; x += 2)
    {
      arduboy.drawPixel(x, d, BLACK);
      arduboy.drawPixel(x + 1, d + 1, BLACK);
    }
  }
}


// the score ///////////////////////////////////////////////////////////////////
// the digits come from the sheet in digits.h (9 x 8 with their own 1 px outline). They overlap 2 px (DIGIT_STEP 7), so they are
// drawn from right to left: every digit covers the left edge of the one to its right.
#define SCORE_X         3
#define SCORE_Y         55      // bottom left corner
#define DIGIT_STEP      7

// a number from the digit sheet, right to left (the digits overlap), the left edge at x or, when centred, the middle
void drawNumber(int16_t x, int16_t y, uint32_t v, bool centred)
{
  char d[8]; byte n = 0;
  do { d[n++] = v % 10; v /= 10; } while (v && n < 7);       // digits, last one first
  if (centred) x -= ((n - 1) * DIGIT_STEP + 9) / 2;
  for (byte k = 0; k < n; k++)                              // right to left
    sprites.drawPlusMask(x + (n - 1 - k) * DIGIT_STEP, y, digits_plus_mask, d[k]);
}

void drawScore() { drawNumber(SCORE_X, SCORE_Y, player.score, false); }

// the points of a fruit float up from the cell where it was eaten
static void drawPopup()
{
  if (!popTimer) return;
  drawNumber(-8 * popCY - 8 - camSX + 16 * popCX + 12, 8 * popCY - camY - 4 - (POP_TIME - popTimer) / 3, popPts, true);
}

// the blast of the bomb: an ellipse (2 : 1) that grows from Yum Yum. It is a circle of radius r with every x doubled.
static inline void plotBlast(uint8_t *buf, int16_t x, int16_t y, bool white)
{
  if ((uint16_t)x > 127 || (uint16_t)y > 63) return;
  uint8_t &b = buf[(y >> 3) * 128 + x];
  if (white) b |= 1 << (y & 7);
  else       b &= ~(1 << (y & 7));
}

static void blastRing(int16_t cx, int16_t cy, int16_t r, bool white)
{
  uint8_t *buf = arduboy.getBuffer();
  int16_t x = 0, y = r, d = 3 - 2 * r;
  while (y >= x)
  {
    for (byte k = 0; k < 8; k++)
    {
      int16_t a = (k & 4) ? y : x, b = (k & 4) ? x : y;                          // (a, b) and (b, a)
      if (k & 1) a = -a;
      if (k & 2) b = -b;
      plotBlast(buf, cx + 2 * a, cy + b, white);
      plotBlast(buf, cx + 2 * a + 1, cy + b, white);
    }
    x++;
    if (d > 0) { y--; d += 4 * (x - y) + 10; }
    else       d += 4 * x + 6;
  }
}

static void drawBlast()
{
  if (!blastR) return;
  int16_t cx = unitScreenX(blastX, blastY), cy = unitScreenY(blastY) - 4, r = blastR >> 1;
  blastRing(cx, cy, r + 1, false);                                               // black edge outside and inside, so it shows on the white walls too
  blastRing(cx, cy, r - 2, false);
  blastRing(cx, cy, r, true);
  blastRing(cx, cy, r - 1, true);
}

// the time that is left of the hop or DOTS: a white line (2 px) on a black strip (4 px) at the right, it gets shorter
static void drawPowerBar()
{
  if (!powerTime) return;
  arduboy.fillRect(124, 0, 4, 64, BLACK);
  byte h = ((uint32_t)powerTime * 64) / POWER_TIME;
  if (h) arduboy.fillRect(125, 64 - h, 2, h, WHITE);
}

void drawPlayField()
{
  drawMaze();
  drawBlast();
  drawPopup();
  drawPowerBar();
  drawScore();
}

#endif
