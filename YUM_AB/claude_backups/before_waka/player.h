#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"
#include "maze.h"

/*
  YUM YUM
  ------------------------------------------------------------------------
  Positions are in 'units': one cell is 16 units wide AND 16 units high, the centre of cell (cx, cy) is
  (16 cx + 8, 16 cy + 8). On the screen X units are pixels, Y units are half pixels (a cell is 8 px high),
  so walking one cell takes the same time in every direction.

  Yum Yum moves 1 unit at a time (several times a frame, see PLAYER_SPEED) and can only turn when it
  is exactly on the centre of a cell. A turn that is asked for a little too early is remembered for
  TURN_BUFFER frames, turning back is always allowed.
*/

#define PLAYER_SPEED    24      // 1/16 unit per frame: 1.5 units per frame = about 4 cells per second
#define TURN_BUFFER     14      // frames a direction stays asked for after the button was pressed
#define START_LIVES     1       // one life: touch a ghost and it is over (set to 3 for the old behaviour)
#define DOT_SCORE       10
#define HIT_FREEZE      36      // frames the game stands still after a hit (the death animation: 6 frames of 6)
#define PELLET_SCORE    50
#define SCARED_TIME     360     // frames the ghosts are frightened (8 seconds)
#define START_CX        7       // start cell (a node, about the middle)
#define START_CY        1

const int8_t dirDX[4] = { 0, -1,  0, 1 };
const int8_t dirDY[4] = { 1,  0, -1, 0 };

static inline bool openDir(int16_t cx, int16_t cy, byte d)
{
  return !isWall(cx + dirDX[d], cy + dirDY[d]);
}

byte freezeTime = 0;             // counts down after a hit
uint16_t scaredTime = 0;         // frames the ghosts are still frightened (after a power pellet)
byte ghostCombo = 0;             // ghosts eaten with this pellet (200, 400, 800, 1600 points)

struct YumYum
{
  int16_t  px, py;              // position in units
  int16_t  bestRow;             // furthest cell row reached
  uint32_t score;
  byte     dir, want, wantTimer;
  byte     acc;                 // 1/16 unit left over from the last frame
  byte     anim;
  byte     lives;
  byte     hurt;                // frames left of the blinking after a hit (can not be hit again)
  bool     started;             // waits for the first button
  bool     moved;               // moved this frame

  void set()
  {
    px = START_CX * 16 + 8;
    py = START_CY * 16 + 8;
    bestRow = START_CY;
    score = 0;
    dir = want = DIR_DOWN;
    wantTimer = 0;
    acc = 0;
    anim = 0;
    lives = START_LIVES;
    hurt = 0;
    started = false;
    moved = false;
    scaredTime = 0;
    ghostCombo = 0;
  }

  void ask(byte d)              // a direction button is down
  {
    want = d;
    wantTimer = TURN_BUFFER;
    started = true;
  }

  int16_t cellX() { return px >> 4; }
  int16_t cellY() { return py >> 4; }

  void eat(int16_t cx, int16_t cy)
  {
    uint8_t slot = cy & (MAZE_RING - 1);
    if ((dotRow[slot] >> cx) & 1)
    {
      dotRow[slot] &= ~(1UL << cx);
      score += DOT_SCORE;
    }
    if ((pelletRow[slot] >> cx) & 1)                                    // power pellet: the ghosts are afraid
    {
      pelletRow[slot] &= ~(1UL << cx);
      score += PELLET_SCORE;
      scaredTime = SCARED_TIME;
      ghostCombo = 0;
    }
  }

  void step()                   // one unit
  {
    bool centre = ((px & 15) == 8) && ((py & 15) == 8);
    int16_t cx = px >> 4, cy = py >> 4;
    if (want != dir)
    {
      if (((want ^ dir) & 1) == 0) dir = want;                        // straight back: any time
      else if (centre && openDir(cx, cy, want)) dir = want;           // round the corner: only in the middle of a cell
    }
    if (centre)
    {
      eat(cx, cy);
      if (!openDir(cx, cy, dir)) return;                              // wall ahead: wait here
    }
    if (dir & 1) px += (dir == DIR_RIGHT) ? 1 : -1;
    else         py += (dir == DIR_DOWN) ? 1 : -1;
    moved = true;
  }

  void update()
  {
    if (hurt) hurt--;
    if (wantTimer && --wantTimer == 0) want = dir;                    // nobody asks for a turn any more
    moved = false;
    if (!started) return;
    acc += PLAYER_SPEED;
    byte n = acc >> 4;
    acc &= 15;
    while (n--) step();
    if (moved && ++anim >= 12) anim = 0;                              // 12 steps = 6 mouth frames, forth and back
    int16_t r = py >> 4;
    if (r > bestRow) bestRow = r;
  }

  byte frame()                  // closed - 1 - 2 - open - 2 - 1
  {
    static const byte seq[6] = { 0, 1, 2, 3, 2, 1 };
    return dir * 4 + seq[anim >> 1];
  }
} player;

#endif
