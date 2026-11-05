#ifndef MAZE_H
#define MAZE_H

#include "globals.h"
#include "chunks.h"

// the difficulty chosen in the menu after PLAY: 0 = EASY, 1 = NORMAL, 2 = HARD (what is chosen last stays)
byte difficulty = 1;
//                                         EASY NORMAL HARD
static const byte diffPellet[3] PROGMEM = { 22, 14,  8 };   // % big ball per side of a node row (more = more ghost hunting)
static const byte diffItem[3][3] PROGMEM = {                // % spring, bomb and DOTS (vanish walls)
  { 10,  7,  4 },
  {  8,  6,  3 },
  {  6,  4,  2 } };
static const byte diffBallGap[3] PROGMEM = { 12, 16, 24 };  // big balls are at least this many cell rows apart (left and right together)
static const byte diffGhosts[3] PROGMEM = {  2,  3,  4 };   // ghosts at the start
static const byte diffGhostRows[3] PROGMEM = { 30, 25, 18 }; // one more ghost every so many rows
static const byte diffGhostMax[3] PROGMEM = {  3,  5,  7 };  // never more than this

/*
  THE ENDLESS MAZE
  ------------------------------------------------------------------------
  The maze is a grid of cells, 17 wide and endless going DOWN. Every cell is a wall or a path.
  A cell is 16 px wide and 8 px high on the screen, and every row slides 4 px to the left
  (the X axis is horizontal, the Y axis runs "down-left", see playfield.h).

      x:  0 1 2 3 4 ... 14 15 16       x = 0 and x = 16 : the border wall
  y odd   # N h N h ... h N #      N = node (always a path), h = passage between two nodes (path or wall)
  y even  # v # v # ... # v #      v = passage between two nodes in the rows above / below, # = pillar (always wall)

  So there are 8 nodes per row. The maze is put together from chunks of 8 node rows that were drawn by hand (tools/chunks.txt,
  checked by tools/chunks.py): every node has at least 2 exits (no dead ends, you can always run in a circle), you can always
  go on downwards, and no pillar is left alone (the wall tiles have no single pillar tile).

  The maze costs 2 x 32 long words: for each cell row one of wall bits and one of dot bits.
*/

#define MAZE_W        17        // cells across (twice as wide as the first version)
#define MAZE_COLS     8         // nodes per row (MAZE_W = 2 * MAZE_COLS + 1)
#define MAZE_RING     32        // cell rows kept in memory (power of 2)
#define ROW_SHIFT     1024      // the world is shifted back by this many cell rows from time to time (see game.h); multiple of MAZE_RING

#ifndef P_PELLET
#define P_PELLET      14        // % chance the left and the right node of a node row (from row PELLET_FROM on) hold a power pellet
#endif
#define PELLET_FROM   9
#ifndef P_SPRING
#define P_SPRING      7         // % chance of a spring (power-up JUMP) in the middle of a node row (from row SPRING_FROM on)
#endif
// power-ups (spring, bomb, DOTS) are at least this many cell rows apart
#define ITEM_GAP      14
#define FRUIT_FROM    6         // fruits (points) from this cell row on
#define P_FRUIT       31        // chance of a fruit in a node row (of 256, so 12 %) ...
#define FRUIT_GAP     24        // ... that is at least this many cell rows after the last one
#define FRUIT_CLEAR   4         // fruits, big balls and items keep this many cell rows apart (the fruit is wider than a node)
#define SPRING_FROM   12
#ifndef P_BOMB
#define P_BOMB        6         // % chance of a bomb (power-up) in the middle of a node row that has no spring (from row BOMB_FROM on)
#endif
#define BOMB_FROM     16
#ifndef P_DOTS
#define P_DOTS        4         // % chance of the power-up DOTS (the walls turn into dots for a while) in the middle of a node row with no other power-up
#endif
#define DOTS_FROM     20


// The maze is stored in 8 bits per row (the rest of a row is fixed: borders, pillars and nodes) instead of 17 bits per cell row:
//   wallB[y]   node row (y odd):    bit k = passage k (between node k and k + 1) is open
//              pillar row (y even): bit c = the passage down from node c is open
//   eatenB[y]  bit = the dot of this cell is gone (eaten, or a power pellet / spring sits there). bit (x >> 1) for x odd, 7 + (x >> 1) for x even
//   pelletB[y] node rows only: bit 0 = pellet at x = 1, bit 1 = pellet at x = 15, bit 2 = spring at x = 7, bit 3 = spring at x = 9, bit 4 = bomb at x = 7, bit 5 = bomb at x = 9, bit 6 = dots at x = 7, bit 7 = dots at x = 9
uint8_t  wallB[MAZE_RING];
uint16_t eatenB[MAZE_RING];
uint8_t  pelletB[MAZE_RING];
uint8_t  fruitB[MAZE_RING];     // a fruit in a node row: bit 7 = there is one, bits 3-4 = the kind (0..3), bits 0-2 = the node column

int16_t camY = 0;               // world Y (px) at the top of the screen
int16_t camSX = 0;              // screen x shift of the camera
int16_t deadY = 0;              // world Y (px): everything above this is the black dead zone

struct
{
  int16_t y;                    // cell row of the next node row (always odd)
  uint8_t chunkId;              // the chunk that is being used, the row in it and if it is mirrored
  uint8_t chunkRow;
  bool    mirror;
  uint16_t n;                   // node rows made so far (the fruits come from this, not from mrnd(): the walls stay the same)
  int16_t lastFruit;            // cell row of the last fruit
  int16_t lastBall;             // cell row of the last big ball (left or right)
  int16_t lastItem;             // cell row of the last spring / bomb / DOTS
} gen;


// random /////////////////////////////////////////////////////////////////////

uint16_t rngState = 0xACE1;

uint8_t rnd()
{
  rngState ^= rngState << 7;
  rngState ^= rngState >> 9;
  rngState ^= rngState << 8;
  return (uint8_t)(rngState ^ (rngState >> 8));
}


// the maze has a random generator of its own: the same seed always gives the same maze
// (whatever the ghosts and the player do with rnd())
uint16_t mazeSeed = 1;
uint16_t mazeRng = 1;

uint8_t mrnd()
{
  mazeRng ^= mazeRng << 7;
  mazeRng ^= mazeRng >> 9;
  mazeRng ^= mazeRng << 8;
  return (uint8_t)(mazeRng ^ (mazeRng >> 8));
}


// queries ////////////////////////////////////////////////////////////////////

// ONE timer for the two timed power-ups, the hop (spring) and DOTS: powerTime counts down from POWER_TIME (it is shown as the bar on the left).
// the power-up DOTS: for powerTime frames all walls except the border are dots that Yum Yum can walk through.
// wallsOff: they can be walked through (it stays on after the time is up until Yum Yum is back on a real path cell),
// wallsHide: they are drawn as dots (they come back for a moment before the end, as a warning)
#define POWER_TIME      360     // frames (8 seconds)
uint16_t powerTime = 0;
bool wallsOff = false;
bool wallsHide = false;

// the wall bit of a cell inside the maze (see the pictures above: x odd = node or passage down, x even = pillar or passage sideways)
static bool rawWall(uint8_t x, int16_t y)
{
  uint8_t b = wallB[y & (MAZE_RING - 1)];
  if (y & 1)                                          // node row
  {
    if (x & 1) return false;                          // a node
    if (x == 0 || x == MAZE_W - 1) return true;       // the border
    return !((b >> ((x >> 1) - 1)) & 1);              // passage sideways
  }
  if (!(x & 1)) return true;                          // pillar
  return !((b >> (x >> 1)) & 1);                      // passage down
}

static inline bool dotsOpen(uint8_t x, int16_t y) { return x != 0 && x != MAZE_W - 1 && y > 0; }   // (not the border)

static bool cellWall(uint8_t x, int16_t y)
{
  return rawWall(x, y) && !(wallsOff && dotsOpen(x, y));
}

// the wall bit for drawing: outside the maze there is nothing (used to pick the wall tiles)
static bool wallBit(int16_t x, int16_t y)
{
  if (x < 0 || x >= MAZE_W || y < 0) return false;
  return rawWall(x, y) && !(wallsHide && dotsOpen(x, y));
}

// for the movers: outside the maze is wall
static bool isWall(int16_t x, int16_t y)
{
  if (x < 0 || x >= MAZE_W || y < 0) return true;
  return cellWall(x, y);
}

static inline uint8_t eatenBit(uint8_t x) { return (x & 1) ? (x >> 1) : 7 + (x >> 1); }

static bool dotAt(int16_t x, int16_t y)
{
  if (x < 0 || x >= MAZE_W || y < 0 || cellWall(x, y)) return false;
  return !((eatenB[y & (MAZE_RING - 1)] >> eatenBit(x)) & 1);
}

static inline void eatDot(int16_t x, int16_t y) { eatenB[y & (MAZE_RING - 1)] |= 1U << eatenBit(x); }

// which bit of pelletB[] belongs to column x (0 = no pellet can be there)
static inline uint8_t pelletMask(int16_t x) { return (x == 1) ? 1 : (x == MAZE_W - 2) ? 2 : (x == 7) ? 0x54 : (x == 9) ? 0xA8 : 0; }

// a fruit sits here: 0 = no, else its kind + 1
static uint8_t fruitAt(int16_t x, int16_t y)
{
  uint8_t f = fruitB[y & (MAZE_RING - 1)];
  return ((y & 1) && (x & 1) && (f & 0x80) && (f & 7) == (x >> 1)) ? ((f >> 3) & 3) + 1 : 0;
}

static bool pelletAt(int16_t x, int16_t y)
{
  return (y & 1) && (pelletB[y & (MAZE_RING - 1)] & pelletMask(x));
}

// the power-up DOTS sits here (the dots bits are 6 and 7)
static bool dotsAt(int16_t x, int16_t y)
{
  return (y & 1) && (pelletB[y & (MAZE_RING - 1)] & pelletMask(x) & 0xC0);
}

// a bomb (power-up) sits here (only x = 7 or 9: the bomb bits are 4 and 5)
static bool bombAt(int16_t x, int16_t y)
{
  return (y & 1) && (pelletB[y & (MAZE_RING - 1)] & pelletMask(x) & 0x30);
}

static inline int16_t deadRow() { return deadY >> 3; }          // cell rows above this one are black


// generation /////////////////////////////////////////////////////////////////
//
// The maze is built from CHUNKS (chunks.h, drawn in tools/chunks.txt): 8 node rows each, 2 bytes per node row (the passages sideways and
// the passages down, see below). A chunk starts with a full corridor, so any chunk fits under any chunk. A new chunk is picked by random,
// never the same one twice in a row, and it is mirrored left <-> right by random.

static uint8_t revBits(uint8_t v, uint8_t n)
{
  uint8_t r = 0;
  while (n--) { r = (r << 1) | (v & 1); v >>= 1; }
  return r;
}

void genNodeRow()
{
  if (gen.chunkRow == 0)                                                                  // the next chunk
  {
    uint8_t id = mrnd() % CHUNKS;
    if (id == gen.chunkId) id = (id + 1) % CHUNKS;
    gen.chunkId = id;
    gen.mirror = mrnd() & 1;
  }
  const uint8_t *p = chunkData + (gen.chunkId * CHUNK_ROWS + gen.chunkRow) * 2;
  uint8_t h = pgm_read_byte(p), down = pgm_read_byte(p + 1);
  if (gen.mirror) { h = revBits(h, MAZE_COLS - 1); down = revBits(down, MAZE_COLS); }
  if (++gen.chunkRow == CHUNK_ROWS) gen.chunkRow = 0;

  // write the two cell rows (see the top of this file for the format)
  uint8_t r0 = gen.y & (MAZE_RING - 1), r1 = (gen.y + 1) & (MAZE_RING - 1);
  wallB[r0] = h;
  wallB[r1] = down;
  eatenB[r0] = 0;
  eatenB[r1] = 0;
  fruitB[r0] = 0;
  fruitB[r1] = 0;
  pelletB[r0] = 0;
  pelletB[r1] = 0;
  if (gen.y >= PELLET_FROM)
    for (uint8_t c = 0; c < MAZE_COLS; c += MAZE_COLS - 1)                       // power pellets only in the first and the last node column
      if ((gen.y - gen.lastBall) >= pgm_read_byte(&diffBallGap[difficulty]) && (gen.y - gen.lastFruit) >= FRUIT_CLEAR && mrnd() % 100 < pgm_read_byte(&diffPellet[difficulty]))
      {
        gen.lastBall = gen.y;
        pelletB[r0] |= (c == 0) ? 1 : 2;
        eatenB[r0]  |= 1U << c;
      }
  bool itemRoom = (gen.y - gen.lastItem) >= ITEM_GAP && (gen.y - gen.lastFruit) >= FRUIT_CLEAR;
  static const byte itemFrom[3] PROGMEM = { SPRING_FROM, BOMB_FROM, DOTS_FROM };   // a spring, else a bomb, else DOTS: in node 3 or 4 (x = 7 or 9)
  for (uint8_t i = 0; i < 3 && itemRoom; i++)
    if (gen.y >= pgm_read_byte(&itemFrom[i]) && mrnd() % 100 < pgm_read_byte(&diffItem[i][difficulty]))
    {
      uint8_t c = 3 + (mrnd() & 1);
      pelletB[r0] |= (4 << (2 * i)) << (c - 3);                                   // 4 / 8 spring, 16 / 32 bomb, 64 / 128 DOTS
      eatenB[r0]  |= 1U << c;
      break;
    }

  if (pelletB[r0] & 0xFC) gen.lastItem = gen.y;

  gen.n++;
  if (gen.y >= FRUIT_FROM && (gen.y - gen.lastFruit) >= FRUIT_GAP && (gen.y - gen.lastItem) >= FRUIT_CLEAR && (gen.y - gen.lastBall) >= FRUIT_CLEAR)               // a fruit
  {
    uint8_t hs = (uint8_t)gen.n * 73 + (uint8_t)mazeSeed * 29 + (uint8_t)(mazeSeed >> 8);      // (a hash, no mrnd(): the walls and power-ups stay as they were)
    hs ^= hs >> 3; hs *= 45;
    if (hs < P_FRUIT)
    {
      uint8_t kind = gen.n / 40 + ((hs >> 2) & 1);                               // deeper in the maze: better fruit
      if (kind > 3) kind = 3;
      uint8_t col = hs & 3;                                                      // node column 1, 2, 5 or 6 (the others hold power-ups)
      fruitB[r0] = 0x80 | (kind << 3) | (col + 1 + (col > 1 ? 2 : 0));
      gen.lastFruit = gen.y;
    }
  }

  gen.y += 2;
}

// make sure all cell rows up to y exist
void ensureRows(int16_t y)
{
  while (gen.y <= y) genNodeRow();
}

// seed 0 is not a seed (it would never change): it becomes 1
void mazeInit(uint16_t seed)
{
  mazeSeed = seed ? seed : 1;
  powerTime = 0;
  wallsOff = false;
  wallsHide = false;
  mazeRng = mazeSeed;
  for (uint8_t i = 0; i < 8; i++) mrnd();
  for (uint8_t i = 0; i < MAZE_RING; i++) { wallB[i] = 0; eatenB[i] = 0; pelletB[i] = 0; fruitB[i] = 0; }   // (row 0 = all passages closed = the wall on top of the maze)
  gen.chunkId = 255;
  gen.chunkRow = 0;
  gen.y = 1;
  gen.lastItem = -100;
  gen.lastBall = -100;
  gen.lastFruit = -100;
  gen.n = 0;
  ensureRows(20);
}

#endif
