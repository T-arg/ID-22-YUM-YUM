#ifndef ENEMIES_H
#define ENEMIES_H

#include "globals.h"
#include "maze.h"
#include "player.h"

/*
  THE GHOSTS
  ------------------------------------------------------------------------
  They walk the maze like Yum Yum does: 1 unit at a time, and they choose a direction at the centre of
  every cell. They never turn round (unless there is no other way) and most of the time they take the
  exit that gets them closest to Yum Yum. New ghosts turn up below the screen, ghosts that are left far
  behind (or are eaten by the dead zone) disappear. The longer you play, the more of them there are.
*/

#define MAX_GHOSTS      8
#define GHOST_SPEED     20      // 1/16 unit per frame (Yum Yum: 24)
#define GHOSTS_FROM_ROW 14      // no ghosts in the first rows
#define SPAWN_AHEAD     9       // new ghosts show up this many cell rows below Yum Yum

struct Ghost
{
  int16_t px, py;
  byte    dir, acc, anim, face;   // face: 0 = looks right, 1 = looks left
  byte    eyeT;                   // how long the eyes have been on their way (in 8 frames)
  bool    active;
  bool    eyes;                   // eaten: only the eyes are left, and they go back through the maze to the dead zone
};

Ghost ghosts[MAX_GHOSTS];
byte  spawnDelay = 0;

void ghostsClear(byte delayFrames)
{
  for (byte i = 0; i < MAX_GHOSTS; i++) ghosts[i].active = false;
  spawnDelay = delayFrames;
  scaredTime = 0;
}

static byte ghostsWanted()
{
  if (player.bestRow < GHOSTS_FROM_ROW) return 0;
  byte n = 3 + (player.bestRow - GHOSTS_FROM_ROW) / 25;          // 3 ghosts at first, one more every 25 rows
  return (n > MAX_GHOSTS) ? MAX_GHOSTS : n;
}

static void ghostSpawn(Ghost &g)
{
  int16_t y = (int16_t)((player.py >> 4) + SPAWN_AHEAD + (rnd() % 3)) | 1;      // a node row
  ensureRows(y + 1);
  int16_t x = 2 * (rnd() & (MAZE_COLS - 1)) + 1;                                               // a node column
  g.px = x * 16 + 8;
  g.py = y * 16 + 8;
  g.dir = rnd() & 3;
  g.face = rnd() & 1;
  g.acc = 0;
  g.anim = 0;
  g.eyes = false;
  g.active = true;
}

// the way back for the eyes ///////////////////////////////////////////////////
#define EYES_SPEED      48      // 1/16 unit per frame: 3 units per frame, much faster than the ghosts
#define EYES_ROWS       12      // node rows that are looked at (a window of 23 cell rows)

static inline bool cellOpen(int16_t x, int16_t y)
{
  return y < gen.y && !isWall(x, y);                                             // (rows that are not made yet are walls)
}

// The eyes stand on the node (cx, cy). Which way to the top of the window (= the dead zone)?
// A breadth first search over the nodes of the window, starting at the top row.
__attribute__((noinline)) static byte eyesDir(int16_t cx, int16_t cy, byte dir)
{
  int16_t top = deadRow() + 1;
  if (cy - 20 > top) top = cy - 20;
  top |= 1;                                                                      // a node row
  uint8_t dist[EYES_ROWS * MAZE_COLS];
  int16_t jj = (cy - top) >> 1, cc = cx >> 1;
  if (jj < 0 || jj >= EYES_ROWS) return dir;                                     // (not in the window: go on)
  byte here = jj * MAZE_COLS + cc;
  for (byte i = 0; i < EYES_ROWS * MAZE_COLS; i++) dist[i] = 255;
  for (byte c = 0; c < MAZE_COLS; c++) dist[c] = 0;
  for (byte lvl = 0; lvl < 250; lvl++)
  {
    bool any = false;
    for (byte i = 0; i < EYES_ROWS * MAZE_COLS; i++)
    {
      if (dist[i] != lvl) continue;
      any = true;
      byte c = i & (MAZE_COLS - 1), j = i >> 3;
      int16_t x = 2 * c + 1, y = top + 2 * j;
      if (c > 0 && cellOpen(x - 1, y) && dist[i - 1] == 255) dist[i - 1] = lvl + 1;
      if (c < MAZE_COLS - 1 && cellOpen(x + 1, y) && dist[i + 1] == 255) dist[i + 1] = lvl + 1;
      if (j > 0 && cellOpen(x, y - 1) && dist[i - MAZE_COLS] == 255) dist[i - MAZE_COLS] = lvl + 1;
      if (j < EYES_ROWS - 1 && cellOpen(x, y + 1) && dist[i + MAZE_COLS] == 255) dist[i + MAZE_COLS] = lvl + 1;
    }
    if (!any || dist[here] != 255) break;                                        // (the search is done when it reaches the eyes)
  }
  byte best = 255, bestDir = dir;
  bool found = false;
  for (byte d = 0; d < 4; d++)
  {
    if (!cellOpen(cx + dirDX[d], cy + dirDY[d])) continue;
    int16_t nj = jj + (d == DIR_DOWN) - (d == DIR_UP), nc = cc + (d == DIR_RIGHT) - (d == DIR_LEFT);
    byte nd = (nj < 0 || nj >= EYES_ROWS) ? 254 : dist[nj * MAZE_COLS + nc];
    if (!found || nd < best) { best = nd; bestDir = d; found = true; }
  }
  return bestDir;
}

__attribute__((noinline)) static void ghostStep(Ghost &g)
{
  if (((g.px & 15) == 8) && ((g.py & 15) == 8))                                  // in the middle of a cell: choose
  {
    int16_t cx = g.px >> 4, cy = g.py >> 4;
    if (g.eyes)                                                                  // the eyes: the shortest way back up
    {
      if ((cx & 1) && (cy & 1)) g.dir = eyesDir(cx, cy, g.dir);
      else if (!openDir(cx, cy, g.dir)) g.dir ^= 2;
      if (!openDir(cx, cy, g.dir)) return;
      if (g.dir & 1) { g.px += (g.dir == DIR_RIGHT) ? 1 : -1; g.face = (g.dir == DIR_LEFT); }
      else           g.py += (g.dir == DIR_DOWN) ? 1 : -1;
      return;
    }
    byte opt[4], n = 0;
    for (byte d = 0; d < 4; d++) if (d != (g.dir ^ 2) && openDir(cx, cy, d)) opt[n++] = d;
    if (n == 0) g.dir ^= 2;
    else if (n == 1) g.dir = opt[0];
    else if (scaredTime)                                                         // frightened: run away
    {
      int16_t pcx = player.px >> 4, pcy = player.py >> 4, best = -1;
      for (byte i = 0; i < n; i++)
      {
        int16_t dx = cx + dirDX[opt[i]] - pcx, dy = cy + dirDY[opt[i]] - pcy;
        int16_t dist = abs(dx) + abs(dy);
        if (dist > best) { best = dist; g.dir = opt[i]; }
      }
    }
    else if (rnd() & 7)                                                          // 7 out of 8: chase
    {
      int16_t pcx = player.px >> 4, pcy = player.py >> 4, best = 32767;
      for (byte i = 0; i < n; i++)
      {
        int16_t dx = cx + dirDX[opt[i]] - pcx, dy = cy + dirDY[opt[i]] - pcy;
        int16_t dist = abs(dx) + abs(dy);
        if (dist < best) { best = dist; g.dir = opt[i]; }
      }
    }
    else g.dir = opt[rnd() % n];
    if (!openDir(cx, cy, g.dir)) return;
  }
  if (g.dir & 1) { g.px += (g.dir == DIR_RIGHT) ? 1 : -1; g.face = (g.dir == DIR_LEFT); }
  else           g.py += (g.dir == DIR_DOWN) ? 1 : -1;
}

__attribute__((noinline)) void ghostsUpdate()
{
  if (spawnDelay) spawnDelay--;
  bool justScared = (scaredTime == SCARED_TIME);                                 // a pellet was just eaten: everybody turns round
  if (scaredTime) scaredTime--;
  byte active = 0;
  Ghost *free = 0;
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    if (ghosts[i].active) active++;
    else free = &ghosts[i];
  }
  if (free && active < ghostsWanted() && spawnDelay == 0 && !wallsOff)
  {
    ghostSpawn(*free);
    spawnDelay = 30 + (rnd() & 31);
  }

  byte speed = GHOST_SPEED + ((player.bestRow > 600) ? 6 : player.bestRow / 100);
  int16_t pr = player.py >> 4;
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    Ghost &g = ghosts[i];
    if (!g.active) continue;
    if (justScared && !g.eyes) { g.dir ^= 2; if (g.dir & 1) g.face = (g.dir == DIR_LEFT); }
    g.acc += g.eyes ? EYES_SPEED : (scaredTime ? (speed * 2 / 3) : speed);       // frightened ghosts are slower, the eyes are fast
    byte n = g.acc >> 4;
    g.acc &= 15;
    while (n--) ghostStep(g);
    g.anim++;
    int16_t r = g.py >> 4;
    if (r < deadRow() - 2 || r < pr - 10 || r > pr + 14) g.active = false;      // left behind or eaten by the dead zone
    if (g.eyes && r <= deadRow() + 2) g.active = false;                          // the eyes are back in the dark
    if (g.eyes && (g.anim & 7) == 0 && ++g.eyeT > 60) g.active = false;         // (they never take longer than 8 seconds)
  }
}

// the power-up DOTS ///////////////////////////////////////////////////////////
#define DOTS_TIME       360     // frames (8 seconds)
#define DOTS_WARN       90      // the walls flash back for the last 2 seconds

// picked up: no ghosts, the walls are dots
void dotsStart()
{
  for (byte i = 0; i < MAX_GHOSTS; i++) ghosts[i].active = false;
  scaredTime = 0;
  dotsTime = DOTS_TIME;
  wallsOff = true;
  wallsHide = true;
}

// once per frame. When the time is up the walls only come back when Yum Yum stands in the middle of a real corridor cell
void dotsTick()
{
  if (!wallsOff) return;
  if (dotsTime) dotsTime--;
  wallsHide = dotsTime && !(dotsTime < DOTS_WARN && (dotsTime & 8));
  if (dotsTime || player.jumpRem) return;
  if (((player.px & 15) == 8) && ((player.py & 15) == 8) && !rawWall(player.px >> 4, player.py >> 4))
  {
    wallsOff = false;
    wallsHide = false;
    spawnDelay = 60;                                                             // the ghosts come back after a moment
  }
}

// the bomb ///////////////////////////////////////////////////////////////////
// An ellipse (twice as wide as high, like the floor) grows fast from Yum Yum and kills every ghost it touches.
// It is done when it is wider than the maze. blastR = width of the ellipse / 2 in px, 0 = no blast.
#define BLAST_START     16      // the first ellipse already covers the ghosts next to Yum Yum
#define BLAST_STEP      8       // px per frame
#define BLAST_MAX       300     // wider than the maze (17 cells of 16 px)
#define BLAST_SCORE     200

int16_t blastR = 0;
int16_t blastX = 0, blastY = 0;          // where it was set off (units, the maze scrolls past it)

void blastStart()
{
  player.item = ITEM_NONE;
  blastR = BLAST_START;
  blastX = player.px;
  blastY = player.py;
  if (spawnDelay < 90) spawnDelay = 90;  // no new ghosts while it grows
  sfx(SFX_POWER, 1, 400);
}

__attribute__((noinline)) void blastUpdate()
{
  if (!blastR) return;
  bool last = blastR >= BLAST_MAX;
  int32_t r2 = (int32_t)blastR * blastR;
  int16_t bx = blastX - (blastY >> 1), by = blastY >> 1;                         // screen position without the camera
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    Ghost &g = ghosts[i];
    if (!g.active || g.eyes) continue;
    int32_t dx = (g.px - (g.py >> 1)) - bx, dy = (g.py >> 1) - by;
    if (last || dx * dx + 4 * dy * dy <= r2)                                     // inside the ellipse (the last frame: everybody)
    {
      g.active = false;
      player.score += BLAST_SCORE;
    }
  }
  blastR = last ? 0 : blastR + BLAST_STEP;
}

// is a ghost touching Yum Yum ?
__attribute__((noinline)) bool ghostsHit()
{
  if (player.jumpRem) return false;                                              // in the air: nothing can touch Yum Yum
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    if (!ghosts[i].active || ghosts[i].eyes) continue;
    if (abs(ghosts[i].px - player.px) < 10 && abs(ghosts[i].py - player.py) < 10)
    {
      if (scaredTime)                                                            // frightened: Yum Yum eats the ghost
      {
        ghosts[i].eyes = true;                                                   // only the eyes are left
        ghosts[i].eyeT = 0;
        ghosts[i].acc = 0;
        player.score += 200UL << ghostCombo;
        sfx(SFX_GHOST + ghostCombo, 1, 260);                                    // climbs with every ghost
        if (ghostCombo < 3) ghostCombo++;
        if (spawnDelay < 120) spawnDelay = 120;
        continue;
      }
      if (!player.hurt) return true;
    }
  }
  return false;
}

#endif
