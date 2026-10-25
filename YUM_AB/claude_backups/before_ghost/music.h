#ifndef MUSIC_H
#define MUSIC_H

// Sound effects (ATMlib): plain PROGMEM tracks that end with ATM_STOP_CHAN, a PROGMEM list, played with sfx(number).
// ATMlib has no song here: playSfx() starts the sound interrupt by itself, and sfxUpdate() (once per frame) switches it off again
// when the effect is over, so the maze gets all the processor time.

#include <ATMcmds.h>

// the "waka waka" of Yum Yum eating dots: a falling "wa" and a rising "ka", they take turns
const uint8_t yumWa[] PROGMEM = {
  ATM_VOL(48),
  ATM_NOTE_A3,
  ATM_DELAY(1),
  ATM_NOTE_F3,
  ATM_DELAY(1),
  ATM_NOTE_D3,
  ATM_DELAY(1),
  ATM_STOP_CHAN
};

const uint8_t yumKa[] PROGMEM = {
  ATM_VOL(48),
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
  ATM_VOL(48),
  ATM_NOTE_C3, ATM_DELAY(1),
  ATM_NOTE_E3, ATM_DELAY(1),
  ATM_NOTE_G3, ATM_DELAY(1),
  ATM_NOTE_C3 + 12, ATM_DELAY(1),
  ATM_NOTE_E3 + 12, ATM_DELAY(1),
  ATM_NOTE_G3 + 12, ATM_DELAY(3),
  ATM_STOP_CHAN
};

#define SFX_WA 0
#define SFX_KA 1
#define SFX_POWER 2
const unsigned char * const PROGMEM soundFX[] =
{
  yumWa,
  yumKa,
  yumPower,
};

#define SFX_MS 170                               // 3 ticks of 40 ms and a little extra: then the sound interrupt goes off
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

#endif
