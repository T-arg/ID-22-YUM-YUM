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
#define JUMP_CELLS      2       // a jump takes Yum Yum 2 cells forward: over a wall into the next corridor
#define ITEM_NONE       0       // the power-up Yum Yum carries (one at a time, a new one replaces it)
#define ITEM_SPRING     1       // B: jump
#define ITEM_BOMB       2       // B: blast that kills all ghosts
#define JUMP_ASK        14      // frames the A button is remembered (the jump starts on the next cell centre)
#define START_CX        7       // start cell (a node, about the middle)
#define START_CY        1

void dotsStart();                // (enemies.h)

const int8_t dirDX[4] = { 0, -1,  0, 1 };
const int8_t dirDY[4] = { 1,  0, -1, 0 };

static inline bool openDir(int16_t cx, int16_t cy, byte d)
{
  return !isWall(cx + dirDX[d], cy + dirDY[d]);
}

byte freezeTime = 0;             // counts down after a hit
uint16_t scaredTime = 0;         // frames the ghosts are still frightened (after a power pellet)
byte wakaTimer = 0;                     // frames the "waka waka" goes on after a dot (it follows the mouth)
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
  byte     item;                // the power-up that is carried: ITEM_NONE, ITEM_SPRING or ITEM_BOMB
  byte     jumpAsk;             // B was pressed: jump as soon as possible
  byte     jumpRem;             // units still to fly in a jump (0 = not jumping)

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
    item = ITEM_NONE;
    jumpAsk = 0;
    jumpRem = 0;
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
    if (dotAt(cx, cy))
    {
      eatDot(cx, cy);
      score += DOT_SCORE;
      wakaTimer = 16;                                                   // the "waka waka" keeps going with the mouth for a while
    }
    if (pelletAt(cx, cy))
    {
      uint8_t &pb = pelletB[cy & (MAZE_RING - 1)];
      if (cx == 7 || cx == 9)                                           // a spring, a bomb or DOTS (they only live in the middle of the playfield)
      {
        byte kind = pb & pelletMask(cx);
        pb &= ~pelletMask(cx);
        if (kind & 0xC0) dotsStart();                                   // DOTS works at once and does not take your place
        else item = (kind & 0x30) ? ITEM_BOMB : ITEM_SPRING;            // a spring or a bomb replaces what you carry
        score += PELLET_SCORE;
        sfx(SFX_POWER, 1, 400);
        return;
      }
      pb &= ~pelletMask(cx);
      score += PELLET_SCORE;
      scaredTime = SCARED_TIME;
      sfx(SFX_POWER, 1, 400);                                           // the big ball
      ghostCombo = 0;
    }
  }

  __attribute__((noinline)) void step()                   // one unit
  {
    if (jumpRem) return;                                                // in the air: update() moves Yum Yum
    bool centre = ((px & 15) == 8) && ((py & 15) == 8);
    int16_t cx = px >> 4, cy = py >> 4;
    if (want != dir)
    {
      if (((want ^ dir) & 1) == 0) dir = want;                        // straight back: any time
      else if (centre && openDir(cx, cy, want)) dir = want;           // round the corner: only in the middle of a cell
    }
    if (centre && jumpAsk && item == ITEM_SPRING)                                 // jump: JUMP_CELLS cells forward, over walls and ghosts
    {
      int16_t tx = cx + JUMP_CELLS * dirDX[dir], ty = cy + JUMP_CELLS * dirDY[dir];
      ensureRows(ty + 3);
      if (!isWall(tx, ty))                                              // (it must land in a corridor)
      {
        item = ITEM_NONE;
        jumpAsk = 0;
        jumpRem = JUMP_CELLS * 16;
        return;
      }
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

  __attribute__((noinline)) void update()
  {
    if (hurt) hurt--;
    if (jumpAsk) jumpAsk--;
    if (wantTimer && --wantTimer == 0) want = dir;                    // nobody asks for a turn any more
    moved = false;
    if (!started) return;
    acc += PLAYER_SPEED;
    byte n = acc >> 4;
    acc &= 15;
    while (n--) step();
    if (jumpRem)                                                        // flying: 3 units per frame
    {
      for (byte k = 0; k < 3 && jumpRem; k++)
      {
        if (dir & 1) px += (dir == DIR_RIGHT) ? 1 : -1;
        else         py += (dir == DIR_DOWN) ? 1 : -1;
        jumpRem--;
      }
      moved = true;
    }
    if (moved)
    {
      anim += 3; if (anim >= 24) anim -= 24;
      if (wakaTimer)
      {
        wakaTimer--;
        if (anim == 0)  sfx(SFX_WA);                                  // mouth closed: "wa"
        if (anim == 12) sfx(SFX_KA);                                  // mouth wide open: "ka"
      }
    }             // 8 frames = 6 mouth frames, forth and back (it used to be 12: 50% faster)
    int16_t r = py >> 4;
    if (r > bestRow) bestRow = r;
  }

  byte lift()                   // px above the floor while jumping (a parabola)
  {
    if (!jumpRem) return 0;
    int16_t p = JUMP_CELLS * 16 - jumpRem, t = JUMP_CELLS * 16;
    return (p * (t - p)) / 26;
  }

  byte frame()                  // closed - 1 - 2 - open - 2 - 1
  {
    static const byte seq[6] = { 0, 1, 2, 3, 2, 1 };
    return dir * 4 + seq[anim >> 2];
  }
} player;

#endif
