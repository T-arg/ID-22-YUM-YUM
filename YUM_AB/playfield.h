#ifndef PLAYFIELD_H
#define PLAYFIELD_H

#include "globals.h"
#include "maze.h"
#include "player.h"
#include "enemies.h"
#include "balls.h"
#include "scared.h"
#include "ghost.h"

/*
  DRAWING
  ------------------------------------------------------------------------
  The X axis is horizontal, the Y axis runs down the screen and a little to the left (4 px per cell row):

      screen x = X - Y / 2 - camSX              X, Y : world position in pixels
      screen y = Y - camY                       (a cell is 16 px wide and 8 px high: X = 16 cx, Y = 8 cy)

  Painter's algorithm, from the top row of cells to the bottom one: the tiles of a row, then the ghosts
  and Yum Yum that stand in that row. So a wall in the row below covers the feet of whoever walks in
  front of it. The dead zone is painted over everything at the end.

  A wall tile is 21 x 16: the lower 8 rows are the floor of its own cell, the upper 8 rows are the top
  of the wall, sticking out over the cell above. The floor of the cell is its parallelogram with its
  top left corner at (16 cx - 4 cy, 8 cy); the tile starts 4 px to the left and 8 px above that.
*/

#define FLOOR_GRID      0       // 1 = draw the floor tile (white, with grid lines) on every path cell, 0 = bare black ground (faster)
#define PLAYER_SY       22      // screen y of the centre of Yum Yum's cell
#define SPRITE_LIFT     3       // px the ball floats above the floor
#define DOT_OX          7       // top left of the 6x8 ball sprite inside the tile (the ball is centred on the floor)
#define DOT_OY          7       // (the ball fills rows 0-5 of its sprite: centre = 3,3 -> floor centre 10,12)

byte waveT = 0;                 // animation clock of the bouncing dots (counts game frames)

// height (px) of a dot in the wave: one sine period in 16 steps
const uint8_t PROGMEM dotBounce[16] = { 2, 2, 3, 3, 3, 3, 3, 2, 2, 1, 0, 0, 0, 0, 0, 1 };

// which wall tile for which neighbours: bit 0 = wall above, 1 = below, 2 = left, 3 = right
const uint8_t PROGMEM wallTileOf[16] = { 5, 6, 4, 5, 3, 8, 9, 15, 1, 10, 7, 13, 2, 12, 14, 11 };

static inline uint8_t tileAt(int16_t x, int16_t y)
{
  uint8_t m = (wallBit(x, y - 1) ? 1 : 0) | (wallBit(x, y + 1) ? 2 : 0) | (wallBit(x - 1, y) ? 4 : 0) | (wallBit(x + 1, y) ? 8 : 0);
  return pgm_read_byte(&wallTileOf[m]);
}

// screen position of the centre of the floor at unit position (px, py)
static inline int16_t unitScreenX(int16_t px, int16_t py) { return px - (py >> 2) - camSX; }
static inline int16_t unitScreenY(int16_t py)             { return (py >> 1) - camY; }

static void drawYumYum()
{
  int16_t cx = unitScreenX(player.px, player.py) - 9, cy = unitScreenY(player.py) - 9 - SPRITE_LIFT;
  if (freezeTime)                                                                // a hit: Yum Yum shrinks away
  {
    byte f = (HIT_FREEZE - freezeTime) / 6;
    sprites.drawPlusMask(cx, cy, yumyumDeath_plus_mask, (f > 5) ? 5 : f);
    return;
  }
  if (player.lives == 0) return;                                                 // game over: gone
  if (player.hurt && (player.hurt & 4)) return;                                  // blinks after a hit
  sprites.drawPlusMask(cx, cy, yumyum_plus_mask, player.frame());
}

const uint8_t PROGMEM ghostSeq[4] = { 0, 1, 2, 1 };

static void drawGhostsInRow(int16_t y)
{
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    const Ghost &g = ghosts[i];
    if (!g.active || (g.py >> 4) != y) continue;
    int16_t gx = unitScreenX(g.px, g.py) - 9, gy = unitScreenY(g.py) - 9 - SPRITE_LIFT;
    if (scaredTime && !((scaredTime < 120) ? (scaredTime & 4) : (scaredTime & 16)))   // frightened: blinks between blue and normal (faster the last 2 seconds)
      sprites.drawPlusMask(gx, gy, ghostScared_plus_mask, (g.anim >> 3) & 1);
    else
      sprites.drawPlusMask(gx, gy - 6, ghostBig_plus_mask, g.dir * 4 + ((g.anim >> 3) & 3));     // 18 x 24: the same feet as the 18 x 18 sprites, 6 px higher
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
    int16_t sy  = 8 * y - 8 - camY;
    int16_t sx0 = -4 * y - 4 - camSX;
    int16_t xLo = (-21 - sx0) >> 4, xHi = (127 - sx0) >> 4;                      // only the columns that are on the screen
    if (xLo < 0) xLo = 0;
    if (xHi > MAZE_W - 1) xHi = MAZE_W - 1;
    for (int16_t x = xLo; x <= xHi; x++)
    {
      int16_t sx = sx0 + 16 * x;
      if (sx <= -21 || sx >= 128) continue;
      if (wallBit(x, y)) sprites.drawPlusMask(sx, sy, wallTiles_plus_mask, tileAt(x, y));
      else
      {
#if FLOOR_GRID
        sprites.drawPlusMask(sx, sy, wallTiles_plus_mask, 0);
#endif
        if (pelletAt(x, y))                                                      // power pellet: the big ball, it only blinks
        {
          if (waveT & 8) continue;
          sprites.drawPlusMask(sx + 5, sy + 6, bigBall_plus_mask, 0);               // the ball is 10 x 10 in the top left of its 16 x 16 sprite: centred on the floor              // (stays on the floor, no bounce)
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
// font 3 wide x 5 high: 5 rows per glyph, 3 bits per row (bit 2 = left pixel)
const unsigned char PROGMEM digitFont[10][5] =
{
  {7,5,5,5,7}, {2,6,2,2,7}, {7,1,7,4,7}, {7,1,7,1,7}, {5,5,7,1,1},
  {7,4,7,1,7}, {7,4,7,5,7}, {7,1,2,2,2}, {7,5,7,5,7}, {7,5,7,1,7},
};

const unsigned char PROGMEM letterFont[26][5] =
{
  {2,5,7,5,5}, {6,5,6,5,6}, {7,4,4,4,7}, {6,5,5,5,6}, {7,4,6,4,7}, {7,4,6,4,4}, {7,4,5,5,7},
  {5,5,7,5,5}, {7,2,2,2,7}, {1,1,1,5,2}, {5,5,6,5,5}, {4,4,4,4,7}, {5,7,7,5,5}, {7,5,5,5,5},
  {7,5,5,5,7}, {7,5,7,4,4}, {7,5,5,7,1}, {6,5,6,5,5}, {7,4,7,1,7}, {7,2,2,2,2}, {5,5,5,5,7},
  {5,5,5,5,2}, {5,5,7,7,5}, {5,5,2,5,5}, {5,5,2,2,2}, {7,1,2,4,7},
};

static void drawGlyph(int16_t x, int16_t y, char ch, byte scale, byte color)
{
  const unsigned char *g;
  if (ch >= '0' && ch <= '9') g = digitFont[ch - '0'];
  else if (ch >= 'A' && ch <= 'Z') g = letterFont[ch - 'A'];
  else return;
  for (byte r = 0; r < 5; r++)
  {
    byte bits = pgm_read_byte(g + r);
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
      byte dg = d[n - 1 - i];
      int16_t bx = 4 + i * 4, by = 64 - 2 - 5;
      for (byte r = 0; r < 5; r++)
      {
        byte bits = pgm_read_byte(&digitFont[dg][r]);
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

// lives: a little ball for every life, top left
void drawLives()
{
  for (byte i = 0; i < player.lives; i++)
  {
    arduboy.fillCircle(6 + i * 9, 5, 3, WHITE);
    arduboy.drawCircle(6 + i * 9, 5, 3, BLACK);
  }
}

void drawPlayField()
{
  drawMaze();
  drawScore();
  drawLives();
}

#endif
