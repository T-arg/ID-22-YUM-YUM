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

/*
  DRAWING
  ------------------------------------------------------------------------
  The X axis is horizontal, the Y axis runs down the screen and a little to the left (4 px per cell row):

      screen x = X - Y / 2 - camSX (8 px per cell row)              X, Y : world position in pixels
      screen y = Y - camY                       (a cell is 16 px wide and 8 px high: X = 16 cx, Y = 8 cy)

  Painter's algorithm, from the top row of cells to the bottom one: the tiles of a row, then the ghosts
  and Yum Yum that stand in that row. So a wall in the row below covers the feet of whoever walks in
  front of it. The dead zone is painted over everything at the end.

  A wall tile is 24 x 8 (45 degree parallelogram, no overlap; each row shifts 8 px). OLD: 21 x 16: the lower 8 rows are the floor of its own cell, the upper 8 rows are the top
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

static void drawYumYum()
{
  int16_t cx = unitScreenX(player.px, player.py) - 9, cy = unitScreenY(player.py) - 9 - SPRITE_LIFT - player.lift();
  if (freezeTime)                                                                // a hit: Yum Yum shrinks away
  {
    byte f = (HIT_FREEZE - freezeTime) / 6;
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
    int16_t xLo = (-24 - sx0) >> 4, xHi = (127 - sx0) >> 4;                      // only the columns that are on the screen
    if (xLo < 0) xLo = 0;
    if (xHi > MAZE_W - 1) xHi = MAZE_W - 1;
    for (int16_t x = xLo; x <= xHi; x++)
    {
      int16_t sx = sx0 + 16 * x;
      if (sx <= -24 || sx >= 128) continue;
      if (wallBit(x, y)) sprites.drawPlusMask(sx, sy, wallTiles_plus_mask, tileAt(x, y));
      else
      {
#if FLOOR_GRID
        sprites.drawPlusMask(sx, sy, floorTile_plus_mask, 0);
#endif
        if (pelletAt(x, y) && (x == 7 || x == 9))                                // the spring (power-up JUMP) or the bomb: it hops a little
          sprites.drawPlusMask(sx + 7, sy - 3 - (pgm_read_byte(&dotBounce[((waveT >> 1) + y) & 15]) >> 1), bombAt(x, y) ? bomb_plus_mask : dotsAt(x, y) ? dots_plus_mask : spring_plus_mask, 0);
        else if (pelletAt(x, y))                                                 // power pellet: the big ball, it only blinks
        {
          if (waveT & 8) continue;
          sprites.drawPlusMask(sx + 7, sy - 1, bigBall_plus_mask, 0);               // the ball is 10 x 10 in the top left of its 16 x 16 sprite: centred on the floor              // (stays on the floor, no bounce)
        }
        else if (dotAt(x, y))                                                         // white dots, bouncing in a wave that runs across the maze
        {
          byte lift = pgm_read_byte(&dotBounce[((waveT >> 1) + x * 2 + y) & 15]);
          sprites.drawPlusMask(sx + DOT_OX, sy + DOT_OY - lift, littleBalls_plus_mask, 0);
        }
      }
    }
    drawGhostsInRow(y);
    if (pRow == y) drawYumYum();
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


// text and score /////////////////////////////////////////////////////////////
// font 3 wide x 5 high: the 5 rows of a glyph are packed in one 16 bit number, 3 bits per row, the top row first (bit 2 of a row = left pixel)
// 0 - 9 then A - Z
const uint16_t PROGMEM glyphFont[36] =
{
  0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF,
  0x79EF, 0x7292, 0x7BEF, 0x7BCF, 0x2BED, 0x6BAE,
  0x7927, 0x6B6E, 0x79A7, 0x79A4, 0x796F, 0x5BED,
  0x7497, 0x126A, 0x5BAD, 0x4927, 0x5FED, 0x7B6D,
  0x7B6F, 0x7BE4, 0x7B79, 0x6BAD, 0x79CF, 0x7492,
  0x5B6F, 0x5B6A, 0x5BFD, 0x5AAD, 0x5A92, 0x72A7,
};

// the 15 bits of a character (0 = nothing to draw)
static uint16_t glyphBits(char ch)
{
  if (ch >= '0' && ch <= '9') return pgm_read_word(&glyphFont[ch - '0']);
  if (ch >= 'A' && ch <= 'Z') return pgm_read_word(&glyphFont[10 + ch - 'A']);
  return 0;
}

static void drawGlyph(int16_t x, int16_t y, char ch, byte scale, byte color)
{
  uint16_t g = glyphBits(ch);
  for (byte r = 0; r < 5; r++)
  {
    byte bits = (g >> (12 - 3 * r)) & 7;
    for (byte c = 0; c < 3; c++)
    {
      if (bits & (4 >> c)) arduboy.fillRect(x + c * scale, y + r * scale, scale, scale, color);
    }
  }
}

static void drawText(int16_t x, int16_t y, const char *s, byte scale, byte color)
{
  while (*s) { drawGlyph(x, y, *s++, scale, color); x += 4 * scale; }
}

static int16_t textWidth(const char *s, byte scale)
{
  byte n = 0;
  while (s[n]) n++;
  return n ? n * 4 * scale - scale : 0;
}

static void drawTextCentered(int16_t y, const char *s, byte scale, byte color)
{
  drawText((128 - textWidth(s, scale)) / 2, y, s, scale, color);
}

// the score, white digits with a black outline, bottom left corner
void drawScore()
{
  char d[8]; byte n = 0;
  uint32_t s = player.score;
  do { d[n++] = s % 10; s /= 10; } while (s && n < 7);       // digits, last one first

  for (byte pass = 0; pass < 2; pass++)                       // pass 0: black outline, pass 1: white digits
  {
    for (byte i = 0; i < n; i++)
    {
      uint16_t g = pgm_read_word(&glyphFont[d[n - 1 - i]]);
      int16_t bx = 4 + i * 4, by = 64 - 2 - 5;
      for (byte r = 0; r < 5; r++)
      {
        byte bits = (g >> (12 - 3 * r)) & 7;
        for (byte c = 0; c < 3; c++)
        {
          if (!(bits & (4 >> c))) continue;
          if (pass == 0) arduboy.fillRect(bx + c - 1, by + r - 1, 3, 3, BLACK);
          else           arduboy.drawPixel(bx + c, by + r, WHITE);
        }
      }
    }
  }
}

// the power-up you carry (B uses it), top right
void drawItemHud()
{
  if (player.item) sprites.drawPlusMask(116, 0, (player.item == ITEM_BOMB) ? bomb_plus_mask : spring_plus_mask, 0);
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

void drawPlayField()
{
  drawMaze();
  drawBlast();
  drawItemHud();
  if (wallsOff && dotsTime) arduboy.fillRect(0, 0, dotsTime / 3, 1, WHITE);        // the time that is left of DOTS
  drawScore();
}

#endif
