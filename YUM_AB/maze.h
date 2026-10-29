#ifndef MAZE_H
#define MAZE_H

#include "globals.h"

/*
  THE ENDLESS MAZE
  ------------------------------------------------------------------------
  The maze is a grid of cells, 17 wide and endless going DOWN. Every cell is a wall or a path.
  A cell is 16 px wide and 8 px high on the screen, and every row slides 4 px to the left
  (the X axis is horizontal, the Y axis runs "down-left", see playfield.h).

      x:  0 1 2 3 4 ... 14 15 16       x = 0 and x = 16 : the border wall
  y odd   # N h N h ... h N #      N = node (always a path), h = passage between two nodes (path or wall)
  y even  # v # v # ... # v #      v = passage between two nodes in the rows above / below, # = pillar (always wall)

  So there are 8 nodes per row. Two cell rows (one node row + one passage row) are made at a time with
  Eller's algorithm, which builds a maze row by row and never has to look back:
    - every node always has at least 2 exits: no dead ends, you can always run in a circle
    - cells that belong together are tracked in 'sets', so the maze never gets cut in two pieces
    - a few extra openings are made on purpose, so there are loops
    - no pillar is left alone: all 4 passages around a pillar can never be open (the wall tiles have
      no single pillar tile, and it would be one big empty square anyway)

  The maze costs 2 x 32 long words: for each cell row one of wall bits and one of dot bits.
*/

#define MAZE_W        17        // cells across (twice as wide as the first version)
#define MAZE_COLS     8         // nodes per row (MAZE_W = 2 * MAZE_COLS + 1)
#define MAZE_RING     32        // cell rows kept in memory (power of 2)
#define ROW_SHIFT     1024      // the world is shifted back by this many cell rows from time to time (see game.h); multiple of MAZE_RING

#ifndef P_JOIN
#define P_JOIN        70        // % chance two neighbouring nodes of different sets get a passage
#endif
#ifndef P_LOOP
#define P_LOOP        35        // % chance two neighbouring nodes of the same set get a passage (makes a loop)
#endif
#ifndef P_PELLET
#define P_PELLET      14        // % chance the left and the right node of a node row (from row PELLET_FROM on) hold a power pellet
#endif
#define PELLET_FROM   9
#ifndef P_SPRING
#define P_SPRING      7         // % chance of a spring (power-up JUMP) in the middle of a node row (from row SPRING_FROM on)
#endif
#define SPRING_FROM   12
#ifndef P_BOMB
#define P_BOMB        6         // % chance of a bomb (power-up) in the middle of a node row that has no spring (from row BOMB_FROM on)
#endif
#define BOMB_FROM     16
#ifndef P_DOTS
#define P_DOTS        4         // % chance of the power-up DOTS (the walls turn into dots for a while) in the middle of a node row with no other power-up
#endif
#define DOTS_FROM     20

#ifndef P_DOWN
#define P_DOWN        60        // % chance of an extra passage to the next row
#endif

// The maze is stored in 8 bits per row (the rest of a row is fixed: borders, pillars and nodes) instead of 17 bits per cell row:
//   wallB[y]   node row (y odd):    bit k = passage k (between node k and k + 1) is open
//              pillar row (y even): bit c = the passage down from node c is open
//   eatenB[y]  bit = the dot of this cell is gone (eaten, or a power pellet / spring sits there). bit (x >> 1) for x odd, 7 + (x >> 1) for x even
//   pelletB[y] node rows only: bit 0 = pellet at x = 1, bit 1 = pellet at x = 15, bit 2 = spring at x = 7, bit 3 = spring at x = 9, bit 4 = bomb at x = 7, bit 5 = bomb at x = 9, bit 6 = dots at x = 7, bit 7 = dots at x = 9
uint8_t  wallB[MAZE_RING];
uint16_t eatenB[MAZE_RING];
uint8_t  pelletB[MAZE_RING];

int16_t camY = 0;               // world Y (px) at the top of the screen
int16_t camSX = 0;              // screen x shift of the camera
int16_t deadY = 0;              // world Y (px): everything above this is the black dead zone

struct
{
  uint8_t set[MAZE_COLS];       // set id of the nodes of the next node row
  uint8_t up;                   // bit c: the node c of the next row has a passage to the row above
  uint8_t hPrev;                // bit k: passage k of the previous node row was open
  uint8_t hwy;                  // node rows since the last full-width corridor
  uint8_t vr[MAZE_COLS];        // vertical passages in a row that end at node c (the one from above included)
  int16_t y;                    // cell row of the next node row (always odd)
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

// the power-up DOTS: for dotsTime frames all walls except the border are dots that Yum Yum can walk through.
// wallsOff: they can be walked through (it stays on after the time is up until Yum Yum is back on a real path cell),
// wallsHide: they are drawn as dots (they come back for a moment before the end, as a warning)
uint16_t dotsTime = 0;
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

// exits of node c in the row that is being made: up (from the row above), left and right
static uint8_t nodeDeg(uint8_t c, uint8_t up, uint8_t h)
{
  uint8_t d = (up >> c) & 1;
  if (c > 0)              d += (h >> (c - 1)) & 1;
  if (c < MAZE_COLS - 1)  d += (h >> c) & 1;
  return d;
}

#define MAX_DOWNS     8         // (wanted) at most this many passages down per node row: the maze is more horizontal than vertical
#ifndef P_HIGHWAY
#define P_HIGHWAY     80        // % chance of a full-width corridor (once there have been 3 normal node rows since the last one)
#endif
#define MAX_RUN       3         // at most 3 passages in a row: a straight corridor is never longer than 7 cells

// may passage k of this row be opened (corridor length and the lone pillar rule)?
static bool canOpen(uint8_t h, uint8_t k, uint8_t up)
{
  if ((h >> k) & 1) return true;
  if (((up >> k) & 1) && ((up >> (k + 1)) & 1) && ((gen.hPrev >> k) & 1)) return false;
  int8_t l = 0, r = 0;
  while (k - 1 - l >= 0 && ((h >> (k - 1 - l)) & 1)) l++;
  while (k + 1 + r < MAZE_COLS - 1 && ((h >> (k + 1 + r)) & 1)) r++;
  return l + 1 + r <= MAX_RUN;
}

// may node c get a passage down (corridor length)?
static bool downOK(uint8_t c, uint8_t up)
{
  return !((up >> c) & 1) || gen.vr[c] < MAX_RUN;
}

// 0 = best node to put a passage down (no passage from above, so no corridor gets longer), 1 = longer corridor, 2 = not allowed
static uint8_t downTier(uint8_t c, uint8_t up)
{
  if (!((up >> c) & 1)) return 0;
  return (gen.vr[c] < MAX_RUN) ? 1 : 2;
}

// the nodes of both sides of passage k become one set
static void joinSets(uint8_t k)
{
  uint8_t from = gen.set[k + 1], to = gen.set[k];
  for (uint8_t c = 0; c < MAZE_COLS; c++) if (gen.set[c] == from) gen.set[c] = to;
}

// make the next node row (cell row gen.y) and the passage row below it (cell row gen.y + 1)
// one try at the passages of a node row (uses and changes the sets)
static void makeRow(uint8_t up, uint8_t &h, uint8_t &down)
{
  if (gen.hwy >= 3 && mrnd() % 100 < P_HIGHWAY)                                  // a full-width corridor, with plenty of ways down
  {
    bool ok = true;
    for (uint8_t k = 0; k < MAZE_COLS - 1; k++)                                 // (the lone pillar rule must allow all passages)
      if (((up >> k) & 1) && ((up >> (k + 1)) & 1) && ((gen.hPrev >> k) & 1)) ok = false;
    if (ok)
    {
      h = (1 << (MAZE_COLS - 1)) - 1;
      for (uint8_t k = 0; k < MAZE_COLS - 1; k++) joinSets(k);
      down = 0;
      uint8_t nd = 0;
      for (uint8_t c = 0; c < MAZE_COLS; c++)
        if (downOK(c, up) && (mrnd() % 100 < 75 || nodeDeg(c, up, h) < 2)) { down |= 1 << c; nd++; }
      for (uint8_t t = 0; t < 24 && nd < 5; t++)
      {
        uint8_t c = mrnd() & (MAZE_COLS - 1);
        if (!((down >> c) & 1) && downOK(c, up)) { down |= 1 << c; nd++; }
      }
      return;
    }
  }
  // passages between the nodes of this row
  h = 0;

  // first the nodes that need a way sideways (before the random passages come)
  for (uint8_t c = 0; c < MAZE_COLS; c++)
  {
    bool hasUp = (up >> c) & 1;
    if (hasUp && gen.vr[c] < MAX_RUN && (mrnd() & 1)) continue;                  // a node with a passage from above may get its second exit later
    if (nodeDeg(c, up, h) >= (hasUp ? 2 : 1)) continue;
    bool rightFirst = mrnd() & 1;
    if (c < MAZE_COLS - 1 && nodeDeg(c + 1, up, h) == 0) rightFirst = true;     // join a neighbour that has no exit either
    else if (c > 0 && nodeDeg(c - 1, up, h) == 0) rightFirst = false;
    for (uint8_t t = 0; t < 2; t++)
    {
      bool right = (t == 0) == rightFirst;
      if (right && c < MAZE_COLS - 1 && canOpen(h, c, up)) { h |= 1 << c; joinSets(c); break; }
      if (!right && c > 0 && canOpen(h, c - 1, up)) { h |= 1 << (c - 1); joinSets(c - 1); break; }
    }
  }

  for (uint8_t k = 0; k < MAZE_COLS - 1; k++)
  {
    // the pillar above passage k has its other 3 passages open: this one must stay a wall
    if (!canOpen(h, k, up)) continue;
    bool differ = gen.set[k] != gen.set[k + 1];
    if (mrnd() % 100 < (differ ? P_JOIN : P_LOOP))
    {
      h |= 1 << k;
      if (differ) joinSets(k);
    }
  }

  // a node without any exit gets one sideways (it has no 'up', so no pillar can be in the way)
  for (uint8_t c = 0; c < MAZE_COLS; c++)
  {
    if (nodeDeg(c, up, h) != 0) continue;
    uint8_t k = (c < MAZE_COLS - 1) ? c : c - 1;
    if (!canOpen(h, k, up) && c > 0 && canOpen(h, c - 1, up)) k = c - 1;
    h |= 1 << k;
    joinSets(k);
  }

  // a node that is the end of 2 vertical passages may not go on down, so it needs a way sideways
  for (uint8_t c = 0; c < MAZE_COLS; c++)
  {
    if (downOK(c, up) || nodeDeg(c, up, h) >= 2) continue;
    uint8_t k = (c < MAZE_COLS - 1) ? c : c - 1;
    if (!canOpen(h, k, up) && c > 0 && canOpen(h, c - 1, up)) k = c - 1;
    if (!canOpen(h, k, up)) continue;                                   // (very rare: it gets a long corridor instead, see below)
    if (!((h >> k) & 1)) { h |= 1 << k; joinSets(k); }
  }

  // passages down to the next row
  down = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++) if (nodeDeg(c, up, h) < 2) down |= 1 << c;     // at least 2 exits per node (if it must be, a corridor of 7)

  // Every group of nodes that are linked sideways needs a way down. Then you can get from any node to a
  // lower row without ever going up: there is no pocket that you can only leave by running back up the
  // maze towards the dead zone.
  for (uint8_t c = 0; c < MAZE_COLS; )
  {
    uint8_t e = c;
    while (e < MAZE_COLS - 1 && ((h >> e) & 1)) e++;                                      // the group is c .. e
    bool has = false;
    for (uint8_t d = c; d <= e; d++) if ((down >> d) & 1) has = true;
    if (!has)
    {
      uint8_t best = 3;
      for (uint8_t d = c; d <= e; d++) if (downTier(d, up) < best) best = downTier(d, up);
      uint8_t n = 0;
      for (uint8_t d = c; d <= e; d++) if (downTier(d, up) == best || best == 2) n++;
      uint8_t pick = mrnd() % n;
      for (uint8_t d = c; d <= e; d++) if (downTier(d, up) == best || best == 2) { if (pick == 0) { down |= 1 << d; break; } pick--; }
    }
    c = e + 1;
  }

  uint8_t seen = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++)                                                 // every set must continue downwards
  {
    uint8_t id = gen.set[c];
    if ((seen >> id) & 1) continue;
    seen |= 1 << id;
    uint8_t members = 0, n = 0, any = 0, best = 3;
    bool has = false;
    for (uint8_t d = 0; d < MAZE_COLS; d++)
    {
      if (gen.set[d] != id) continue;
      any |= 1 << d;
      if (downTier(d, up) < best) best = downTier(d, up);
      if ((down >> d) & 1) has = true;
    }
    if (has) continue;
    for (uint8_t d = 0; d < MAZE_COLS; d++) if (((any >> d) & 1) && (downTier(d, up) == best || best == 2)) { members |= 1 << d; n++; }
    uint8_t pick = mrnd() % n;
    for (uint8_t d = 0; d < MAZE_COLS; d++)
    {
      if (!((members >> d) & 1)) continue;
      if (pick == 0) { down |= 1 << d; break; }
      pick--;
    }
  }
  for (uint8_t c = 0; c < MAZE_COLS; c++) if (mrnd() % 100 < P_DOWN && (!((up >> c) & 1) || gen.vr[c] < 2)) down |= 1 << c;

}

// is the row what we want: every node has 2 exits and no corridor is longer than 5 cells?
static bool rowValid(uint8_t up, uint8_t h, uint8_t down, bool strict)
{
  uint8_t limited = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++)
  {
    if (nodeDeg(c, up, h) + ((down >> c) & 1) < 2) return false;
    if (((down >> c) & 1) && !downOK(c, up)) return false;
    if (((down >> c) & 1) && ((up >> c) & 1)) limited++;                       // this node will be at the end of 2 vertical passages
  }
  uint8_t nd = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++) nd += (down >> c) & 1;
  if (strict && (limited > 1 || nd > MAX_DOWNS)) return false;                                        // (it is easier for the next row if there are none)
  uint8_t run = 0;
  if (h == (1 << (MAZE_COLS - 1)) - 1) return true;                                // (the full-width corridor is meant to be long)
  for (uint8_t k = 0; k < MAZE_COLS - 1; k++)
  {
    run = ((h >> k) & 1) ? run + 1 : 0;
    if (run > MAX_RUN) return false;
  }
  return true;
}

void genNodeRow()
{
  uint8_t up = gen.up;

  // nodes that do not come from above start a set of their own
  uint8_t used = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++) if ((up >> c) & 1) used |= 1 << gen.set[c];
  for (uint8_t c = 0; c < MAZE_COLS; c++)
  {
    if ((up >> c) & 1) continue;
    uint8_t id = 0;
    while ((used >> id) & 1) id++;
    gen.set[c] = id;
    used |= 1 << id;
  }

  uint8_t h, down, saved[MAZE_COLS];
  for (uint8_t c = 0; c < MAZE_COLS; c++) saved[c] = gen.set[c];
  for (uint8_t tries = 0; ; tries++)                                                      // (a row that breaks the corridor limit is made again)
  {
    makeRow(up, h, down);
    if (rowValid(up, h, down, tries < 12) || tries >= 60) break;
    for (uint8_t c = 0; c < MAZE_COLS; c++) gen.set[c] = saved[c];
  }

  // write the two cell rows (see the top of this file for the format)
  uint8_t r0 = gen.y & (MAZE_RING - 1), r1 = (gen.y + 1) & (MAZE_RING - 1);
  wallB[r0] = h;
  wallB[r1] = down;
  eatenB[r0] = 0;
  eatenB[r1] = 0;
  pelletB[r0] = 0;
  pelletB[r1] = 0;
  if (gen.y >= PELLET_FROM)
    for (uint8_t c = 0; c < MAZE_COLS; c += MAZE_COLS - 1)                       // power pellets only in the first and the last node column
      if (mrnd() % 100 < P_PELLET)
      {
        pelletB[r0] |= (c == 0) ? 1 : 2;
        eatenB[r0]  |= 1U << c;
      }
  if (gen.y >= SPRING_FROM && mrnd() % 100 < P_SPRING)                          // a spring, only in the middle (node 3 or 4: x = 7 or 9)
  {
    uint8_t c = 3 + (mrnd() & 1);
    pelletB[r0] |= (c == 3) ? 4 : 8;
    eatenB[r0]  |= 1U << c;
  }
  else if (gen.y >= BOMB_FROM && mrnd() % 100 < P_BOMB)                         // a bomb in the same two places
  {
    uint8_t c = 3 + (mrnd() & 1);
    pelletB[r0] |= (c == 3) ? 16 : 32;
    eatenB[r0]  |= 1U << c;
  }
  else if (gen.y >= DOTS_FROM && mrnd() % 100 < P_DOTS)                         // the power-up DOTS
  {
    uint8_t c = 3 + (mrnd() & 1);
    pelletB[r0] |= (c == 3) ? 64 : 128;
    eatenB[r0]  |= 1U << c;
  }

  for (uint8_t c = 0; c < MAZE_COLS; c++)
    gen.vr[c] = ((down >> c) & 1) ? ((((up >> c) & 1) ? gen.vr[c] : 0) + 1) : 0;
  gen.hwy = (h == (1 << (MAZE_COLS - 1)) - 1) ? 0 : (gen.hwy < 250 ? gen.hwy + 1 : 250);
  gen.up = down;
  gen.hPrev = h;
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
  dotsTime = 0;
  wallsOff = false;
  wallsHide = false;
  mazeRng = mazeSeed;
  for (uint8_t i = 0; i < 8; i++) mrnd();
  for (uint8_t i = 0; i < MAZE_RING; i++) { wallB[i] = 0; eatenB[i] = 0; pelletB[i] = 0; }   // (row 0 = all passages closed = the wall on top of the maze)
  gen.up = 0;
  gen.hPrev = 0;
  gen.hwy = 0;
  for (uint8_t c = 0; c < MAZE_COLS; c++) gen.vr[c] = 0;
  gen.y = 1;
  for (uint8_t c = 0; c < MAZE_COLS; c++) gen.set[c] = c;
  ensureRows(20);
}

#endif
