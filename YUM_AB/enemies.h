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

#define MAX_GHOSTS      6
#define GHOST_SPEED     18      // 1/16 unit per frame (Yum Yum: 24)
#define GHOSTS_FROM_ROW 14      // no ghosts in the first rows
#define SPAWN_AHEAD     9       // new ghosts show up this many cell rows below Yum Yum

struct Ghost
{
  int16_t px, py;
  byte    dir, acc, anim, face;   // face: 0 = looks right, 1 = looks left
  bool    active;
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
  byte n = 2 + (player.bestRow - GHOSTS_FROM_ROW) / 40;          // 2 ghosts at first, one more every 40 rows
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
  g.active = true;
}

static void ghostStep(Ghost &g)
{
  if (((g.px & 15) == 8) && ((g.py & 15) == 8))                                  // in the middle of a cell: choose
  {
    int16_t cx = g.px >> 4, cy = g.py >> 4;
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
    else if (rnd() & 3)                                                          // 3 out of 4: chase
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

void ghostsUpdate()
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
  if (free && active < ghostsWanted() && spawnDelay == 0)
  {
    ghostSpawn(*free);
    spawnDelay = 45 + (rnd() & 31);
  }

  byte speed = GHOST_SPEED + ((player.bestRow > 600) ? 6 : player.bestRow / 100);
  int16_t pr = player.py >> 4;
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    Ghost &g = ghosts[i];
    if (!g.active) continue;
    if (justScared) { g.dir ^= 2; if (g.dir & 1) g.face = (g.dir == DIR_LEFT); }
    g.acc += scaredTime ? (speed * 2 / 3) : speed;               // frightened ghosts are slower
    byte n = g.acc >> 4;
    g.acc &= 15;
    while (n--) ghostStep(g);
    g.anim++;
    int16_t r = g.py >> 4;
    if (r < deadRow() - 2 || r < pr - 10 || r > pr + 14) g.active = false;      // left behind or eaten by the dead zone
  }
}

// is a ghost touching Yum Yum ?
bool ghostsHit()
{
  for (byte i = 0; i < MAX_GHOSTS; i++)
  {
    if (!ghosts[i].active) continue;
    if (abs(ghosts[i].px - player.px) < 10 && abs(ghosts[i].py - player.py) < 10)
    {
      if (scaredTime)                                                            // frightened: Yum Yum eats the ghost
      {
        ghosts[i].active = false;
        player.score += 200UL << ghostCombo;
        if (ghostCombo < 3) ghostCombo++;
        sfx(SFX_GHOST, 1, 340);
        if (spawnDelay < 120) spawnDelay = 120;
        continue;
      }
      if (!player.hurt) return true;
    }
  }
  return false;
}

#endif
