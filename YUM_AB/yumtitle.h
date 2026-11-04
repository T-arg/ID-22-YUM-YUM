#ifndef YUMTITLE_H
#define YUMTITLE_H

#include <ATMcmds.h>

#ifndef Song
#define Song const uint8_t PROGMEM
#endif

// Play with:   ATM.play(yumTitle);
// 1 beat = 16 ticks, 1 bar = 64 ticks. Intro: 2 bars, then 16 bars that loop for ever (every channel ends in a GOTO to its own loop track, no hop).
// Channel 3 is not used: ATM_WAVE_CH3 ATM_WAVE_OFF in ATMconfig.h.
// Pitch: ATMlib note names sound two octaves above their label.

// ---------------------------------------------------------------------------
// YUM YUM - title tune: jolly SEGA arcade style, short notes, no drums, 2 bar jingle + 12 bar loop
// C major, tempo 42 (157 BPM), 1 bar = 64 ticks. Intro 2 bars (3 s), loop 12 bars (18 s).
// A: C Am F G | C Em F G7.  B: F G Em Am.  CH0 plucked lead, CH1 arpeggio chords, CH2 plucked bass, CH3 off.
// 456 bytes
// ---------------------------------------------------------------------------
Song yumTitle[] = {          // total song in bytes = 456
  0x0D,                         // Number of tracks
  0x00, 0x00,                   // Address of track 0   //0  lI
  0x36, 0x00,                   // Address of track 1   //54  cI
  0x41, 0x00,                   // Address of track 2   //65  bI
  0x4F, 0x00,                   // Address of track 3   //79  off
  0x50, 0x00,                   // Address of track 4   //80  bb
  0x5B, 0x00,                   // Address of track 5   //91  bar0
  0x5F, 0x00,                   // Address of track 6   //95  bL
  0x91, 0x00,                   // Address of track 7   //145  cL
  0x97, 0x00,                   // Address of track 8   //151  cA
  0xB7, 0x00,                   // Address of track 9   //183  cB
  0xCB, 0x00,                   // Address of track 10   //203  lL
  0xD1, 0x00,                   // Address of track 11   //209  lA
  0x70, 0x01,                   // Address of track 12   //368  lB

  0x00,                         // Channel 0 entry track (lI)
  0x01,                         // Channel 1 entry track (cI)
  0x02,                         // Channel 2 entry track (bI)
  0x03,                         // Channel 3 entry track (off)

  //"Track 0" lI  [54 bytes]
  ATM_SET_TEMPO(42),
  ATM_VOL(40),
  ATM_SL_VOL((uint8_t)-5),
  ATM_NOTE_C3,
  ATM_DELAY(4),
  ATM_NOTE_D3,
  ATM_DELAY(4),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_NOTE_F3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(4),
  ATM_NOTE_D4,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(4),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_G4,
  ATM_DELAY(4),
  ATM_NOTE_A4,
  ATM_DELAY(4),
  ATM_NOTE_B4,
  ATM_DELAY(4),
  ATM_NOTE_A4,
  ATM_DELAY(4),
  ATM_NOTE_G4,
  ATM_DELAY(4),
  ATM_NOTE_F4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(16),
  ATM_GOTO(10),   // -> lL

  //"Track 1" cI  [11 bytes]
  ATM_VOL(16),
  ATM_ARP(0x43, 0x21),
  ATM_NOTE_C2,
  ATM_DELAY(64),
  ATM_NOTE_G2,
  ATM_DELAY(64),
  ATM_GOTO(7),   // -> cL

  //"Track 2" bI  [14 bytes]
  ATM_VOL(58),
  ATM_SL_VOL((uint8_t)-3),
  ATM_SET_TRA(0),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(7),
  ATM_GOTO(5),   // -> bar0
  ATM_GOTO(6),   // -> bL

  //"Track 3" off  [1 bytes]
  ATM_STOP_CHAN,

  //"Track 4" bb  [11 bytes]
  ATM_NOTE_C2,
  ATM_DELAY(8),
  ATM_NOTE_C2,
  ATM_DELAY(4),
  ATM_NOTE_C3,
  ATM_DELAY(4),
  ATM_NOTE_C2,
  ATM_DELAY(8),
  ATM_NOTE_G2,
  ATM_DELAY(8),
  ATM_RETURN,

  //"Track 5" bar0  [4 bytes]
  ATM_REPEAT(1, 4),   // 2x bb
  ATM_RETURN,

  //"Track 6" bL  [50 bytes]
  ATM_SET_TRA(0),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(9),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(5),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(7),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(0),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(4),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(5),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(7),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(5),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(7),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(4),
  ATM_GOTO(5),   // -> bar0
  ATM_SET_TRA(9),
  ATM_GOTO(5),   // -> bar0
  ATM_GOTO(6),   // -> bL

  //"Track 7" cL  [6 bytes]
  ATM_GOTO(8),   // -> cA
  ATM_GOTO(9),   // -> cB
  ATM_GOTO(7),   // -> cL

  //"Track 8" cA  [32 bytes]
  ATM_ARP(0x43, 0x21),
  ATM_NOTE_C3,
  ATM_DELAY(64),
  ATM_ARP(0x34, 0x21),
  ATM_NOTE_A2,
  ATM_DELAY(64),
  ATM_ARP(0x43, 0x21),
  ATM_NOTE_F2,
  ATM_DELAY(64),
  ATM_NOTE_G2,
  ATM_DELAY(64),
  ATM_NOTE_C3,
  ATM_DELAY(64),
  ATM_ARP(0x34, 0x21),
  ATM_NOTE_E2,
  ATM_DELAY(64),
  ATM_ARP(0x43, 0x21),
  ATM_NOTE_F2,
  ATM_DELAY(64),
  ATM_NOTE_G2,
  ATM_DELAY(64),
  ATM_RETURN,

  //"Track 9" cB  [20 bytes]
  ATM_NOTE_F2,
  ATM_DELAY(32),
  ATM_NOTE_F2,
  ATM_DELAY(32),
  ATM_NOTE_G2,
  ATM_DELAY(32),
  ATM_NOTE_G2,
  ATM_DELAY(32),
  ATM_ARP(0x34, 0x21),
  ATM_NOTE_E2,
  ATM_DELAY(32),
  ATM_NOTE_E2,
  ATM_DELAY(32),
  ATM_NOTE_A2,
  ATM_DELAY(32),
  ATM_NOTE_A2,
  ATM_DELAY(32),
  ATM_RETURN,

  //"Track 10" lL  [6 bytes]
  ATM_GOTO(11),   // -> lA
  ATM_GOTO(12),   // -> lB
  ATM_GOTO(10),   // -> lL

  //"Track 11" lA  [159 bytes]
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_A4,
  ATM_DELAY(8),
  ATM_NOTE_F4,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(4),
  ATM_NOTE_G4,
  ATM_DELAY(4),
  ATM_NOTE_B4,
  ATM_DELAY(8),
  ATM_NOTE_A4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_F4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(16),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(4),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_A4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_F4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_B4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(4),
  ATM_NOTE_E4,
  ATM_DELAY(4),
  ATM_NOTE_F4,
  ATM_DELAY(4),
  ATM_NOTE_G4,
  ATM_DELAY(4),
  ATM_NOTE_A4,
  ATM_DELAY(8),
  ATM_NOTE_B4,
  ATM_DELAY(8),
  ATM_RETURN,

  //"Track 12" lB  [57 bytes]
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_F3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(16),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(16),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_B4,
  ATM_DELAY(8),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(16),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_A4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(16),
  ATM_NOTE_C4,
  ATM_DELAY(8),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_RETURN,

};

#endif
