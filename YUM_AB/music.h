#ifndef MUSIC_H
#define MUSIC_H

// Sound effects (ATMlib): plain PROGMEM tracks that end with ATM_STOP_CHAN, a PROGMEM list, played with sfx(number).
// ATMlib has no song here: playSfx() starts the sound interrupt by itself, and sfxUpdate() (once per frame) switches it off again
// when the effect is over, so the maze gets all the processor time.

#include <ATMcmds.h>
#include "yumtitle.h"

// the "waka waka" of Yum Yum eating dots: a falling "wa" and a rising "ka", they take turns
const uint8_t yumWa[] PROGMEM = {
  ATM_SET_TEMPO(50), ATM_VOL(48),
  ATM_NOTE_A3,
  ATM_DELAY(1),
  ATM_NOTE_F3,
  ATM_DELAY(1),
  ATM_NOTE_D3,
  ATM_DELAY(1),
  ATM_STOP_CHAN
};

const uint8_t yumKa[] PROGMEM = {
  ATM_SET_TEMPO(50), ATM_VOL(48),
  ATM_NOTE_D3,
  ATM_DELAY(1),
  ATM_NOTE_F3,
  ATM_DELAY(1),
  ATM_NOTE_A3,
  ATM_DELAY(1),
  ATM_STOP_CHAN
};

// the big ball: a quick rising run, "the ghosts are afraid"
const uint8_t yumPower[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_C3, ATM_DELAY(1),
  ATM_NOTE_E3, ATM_DELAY(1),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_C3 + 12, ATM_DELAY(1),
  ATM_NOTE_E3 + 12, ATM_DELAY(1),
  ATM_NOTE_G3 + 12, ATM_DELAY(3),
  ATM_STOP_CHAN
};

// Yum Yum eats a scared ghost: a chirp that starts higher with every ghost of the same big ball (4 steps)
const uint8_t yumGhost0[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_C3, ATM_DELAY(1),
  ATM_NOTE_E3, ATM_DELAY(1),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_C4, ATM_DELAY(3),
  ATM_STOP_CHAN
};

const uint8_t yumGhost1[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_E3, ATM_DELAY(1),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_C4, ATM_DELAY(1),
  ATM_NOTE_E4, ATM_DELAY(3),
  ATM_STOP_CHAN
};

const uint8_t yumGhost2[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_C4, ATM_DELAY(1),
  ATM_NOTE_E4, ATM_DELAY(1),
  ATM_NOTE_G4, ATM_DELAY(3),
  ATM_STOP_CHAN
};

const uint8_t yumGhost3[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_C4, ATM_DELAY(1),
  ATM_NOTE_E4, ATM_DELAY(1),
  ATM_NOTE_G4, ATM_DELAY(1),
  ATM_NOTE_B4, ATM_DELAY(3),
  ATM_STOP_CHAN
};

// Yum Yum is caught: a falling run (10 ticks) and two beeps, together as long as the death animation (36 frames = 20 ticks)
const uint8_t yumDie[] PROGMEM = {
  ATM_SET_TEMPO(25), ATM_VOL(48),
  ATM_NOTE_B3, ATM_DELAY(1),
  ATM_NOTE_A3, ATM_DELAY(1),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_F3, ATM_DELAY(1),
  ATM_NOTE_E3, ATM_DELAY(1),
  ATM_NOTE_D3, ATM_DELAY(1),
  ATM_NOTE_C3, ATM_DELAY(1),
  ATM_NOTE_B2, ATM_DELAY(1),
  ATM_NOTE_A2, ATM_DELAY(1),
  ATM_NOTE_G2, ATM_DELAY(1),
  ATM_VOL(0), ATM_DELAY(1),                       // a short silence
  ATM_VOL(48), ATM_NOTE_E3, ATM_DELAY(3),         // "biep"
  ATM_VOL(0), ATM_DELAY(2),
  ATM_VOL(48), ATM_NOTE_E3, ATM_DELAY(3),         // "biep"
  ATM_VOL(0), ATM_DELAY(1),
  ATM_STOP_CHAN
};

#define SFX_WA 0
#define SFX_KA 1
#define SFX_POWER 2
#define SFX_DIE 7
#define SFX_GHOST 3                              // 3..6, one per ghost eaten with the same ball
const unsigned char * const PROGMEM soundFX[] =
{
  yumWa,
  yumKa,
  yumPower,
  yumGhost0,
  yumGhost1,
  yumGhost2,
  yumGhost3,
  yumDie,
};

#define SFX_MS 85                                // 3 ticks of 20 ms (tempo 50) and a little extra: then the sound interrupt goes off
unsigned long sfxEnd = 0;

// ATMlib does not look at the Arduboy sound setting, so we do it here
void sfx(byte n, byte ch = 1, unsigned int ms = SFX_MS)
{
  if (!arduboy.audio.enabled()) return;
  ATM.playSfx((const uint8_t *)pgm_read_word(&soundFX[n]), ch);
  sfxEnd = millis() + ms;
}

// once per frame: the effect is over, switch the sound interrupt off
void sfxUpdate()
{
  if (sfxEnd && (long)(millis() - sfxEnd) >= 0)
  {
    ATM.stop();
    sfxEnd = 0;
  }
}

// the title tune (yumTitle: 2 bar fanfare, then a 16 bar loop). It starts at the T-arg logo and plays on the title screen and its sub screens.
// ATM.stop() switches the sound interrupt off again when the game starts: the maze needs the processor.
bool titleOn = false;
void titleMusic(bool on)
{
  if (on && !titleOn && arduboy.audio.enabled())
  {
    ATM.play(yumTitle);
    titleOn = true;
  }
  else if (!on && titleOn)
  {
    ATM.stop();
    titleOn = false;
  }
}

#endif
