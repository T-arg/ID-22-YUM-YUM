#ifndef YUMTITLE_H
#define YUMTITLE_H

#include <ATMcmds.h>

#ifndef Song
#define Song const uint8_t PROGMEM
#endif

// Play with:   ATM.play(yumTitle);
// 1 beat = 16 ticks, 1 bar = 64 ticks. Intro: 2 bars, then 8 bars that loop for ever (every channel ends in a GOTO to its own loop track, no hop).
// Pitch: ATMlib note names sound two octaves above their label.

// ---------------------------------------------------------------------------
// YUM YUM - title tune: heavy bass and drums, arpeggio hook, 2 bar intro + 8 bar loop
// E minor, Em Em C D | Em Em C B, tempo 37 (139 BPM), 1 bar = 64 ticks.
// Intro 2 bars (3.5 s), loop 8 bars (13.8 s).
// CH0 arpeggio hook, CH1 kick on every beat, CH2 heavy saw bass, CH3 noise snare / hats / fills.
// 437 bytes
// ---------------------------------------------------------------------------
Song yumTitle[] = {          // total song in bytes = 437
  0x18,                         // Number of tracks
  0x00, 0x00,                   // Address of track 0   //0  dI
  0x09, 0x00,                   // Address of track 1   //9  dL
  0x15, 0x00,                   // Address of track 2   //21  bar
  0x1E, 0x00,                   // Address of track 3   //30  f4
  0x27, 0x00,                   // Address of track 4   //39  f8
  0x30, 0x00,                   // Address of track 5   //48  hat
  0x43, 0x00,                   // Address of track 6   //67  snare
  0x53, 0x00,                   // Address of track 7   //83  dbl
  0x62, 0x00,                   // Address of track 8   //98  roll
  0x7D, 0x00,                   // Address of track 9   //125  kI
  0x85, 0x00,                   // Address of track 10   //133  kL
  0x91, 0x00,                   // Address of track 11   //145  kick
  0x9C, 0x00,                   // Address of track 12   //156  kghost
  0xAE, 0x00,                   // Address of track 13   //174  q1
  0xBA, 0x00,                   // Address of track 14   //186  q2
  0xC6, 0x00,                   // Address of track 15   //198  bs
  0xCF, 0x00,                   // Address of track 16   //207  bI
  0xDB, 0x00,                   // Address of track 17   //219  bL
  0xFD, 0x00,                   // Address of track 18   //253  hk
  0x08, 0x01,                   // Address of track 19   //264  lI
  0x29, 0x01,                   // Address of track 20   //297  lL
  0x33, 0x01,                   // Address of track 21   //307  l1
  0x5A, 0x01,                   // Address of track 22   //346  l4
  0x6D, 0x01,                   // Address of track 23   //365  l8

  0x13,                         // Channel 0 entry track (lI)
  0x09,                         // Channel 1 entry track (kI)
  0x10,                         // Channel 2 entry track (bI)
  0x00,                         // Channel 3 entry track (dI)

  //"Track 0" dI  [9 bytes]
  ATM_SET_TEMPO(37),
  ATM_REPEAT(3, 5),   // 4x hat
  ATM_GOTO(4),   // -> f8
  ATM_GOTO(1),   // -> dL

  //"Track 1" dL  [12 bytes]
  ATM_REPEAT(2, 2),   // 3x bar
  ATM_GOTO(3),   // -> f4
  ATM_REPEAT(2, 2),   // 3x bar
  ATM_GOTO(4),   // -> f8
  ATM_GOTO(1),   // -> dL

  //"Track 2" bar  [9 bytes]
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(6),   // -> snare
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(6),   // -> snare
  ATM_RETURN,

  //"Track 3" f4  [9 bytes]
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(6),   // -> snare
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(7),   // -> dbl
  ATM_RETURN,

  //"Track 4" f8  [9 bytes]
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(6),   // -> snare
  ATM_GOTO(5),   // -> hat
  ATM_GOTO(8),   // -> roll
  ATM_RETURN,

  //"Track 5" hat  [19 bytes]
  ATM_VOL(0),
  ATM_DELAY(4),
  ATM_VOL(10),
  ATM_SL_VOL((uint8_t)-3),
  ATM_DELAY(4),
  ATM_VOL(24),
  ATM_SL_VOL((uint8_t)-4),
  ATM_DELAY(4),
  ATM_VOL(10),
  ATM_SL_VOL((uint8_t)-3),
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 6" snare  [16 bytes]
  ATM_VOL(52),
  ATM_SL_VOL((uint8_t)-6),
  ATM_DELAY(8),
  ATM_VOL(20),
  ATM_SL_VOL((uint8_t)-4),
  ATM_DELAY(4),
  ATM_VOL(10),
  ATM_SL_VOL((uint8_t)-3),
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 7" dbl  [15 bytes]
  ATM_VOL(52),
  ATM_SL_VOL((uint8_t)-6),
  ATM_DELAY(4),
  ATM_VOL(52),
  ATM_DELAY(4),
  ATM_VOL(52),
  ATM_DELAY(4),
  ATM_VOL(52),
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 8" roll  [27 bytes]
  ATM_VOL(24),
  ATM_SL_VOL((uint8_t)-10),
  ATM_DELAY(2),
  ATM_VOL(28),
  ATM_DELAY(2),
  ATM_VOL(32),
  ATM_DELAY(2),
  ATM_VOL(36),
  ATM_DELAY(2),
  ATM_VOL(40),
  ATM_DELAY(2),
  ATM_VOL(44),
  ATM_DELAY(2),
  ATM_VOL(48),
  ATM_DELAY(2),
  ATM_VOL(52),
  ATM_DELAY(2),
  ATM_RETURN,

  //"Track 9" kI  [8 bytes]
  ATM_DELAY(64),
  ATM_REPEAT(2, 11),   // 3x kick
  ATM_GOTO(12),   // -> kghost
  ATM_GOTO(10),   // -> kL

  //"Track 10" kL  [12 bytes]
  ATM_REPEAT(14, 11),   // 15x kick
  ATM_GOTO(12),   // -> kghost
  ATM_REPEAT(14, 11),   // 15x kick
  ATM_GOTO(12),   // -> kghost
  ATM_GOTO(10),   // -> kL

  //"Track 11" kick  [11 bytes]
  ATM_ARP_OFF,
  ATM_SL_FRQ((uint8_t)-127),
  ATM_VOL(44),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_VOL(0),
  ATM_DELAY(12),
  ATM_RETURN,

  //"Track 12" kghost  [18 bytes]
  ATM_ARP_OFF,
  ATM_SL_FRQ((uint8_t)-127),
  ATM_VOL(44),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_VOL(0),
  ATM_DELAY(6),
  ATM_VOL(26),
  ATM_NOTE_E3,
  ATM_DELAY(2),
  ATM_VOL(0),
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 13" q1  [12 bytes]
  ATM_VOL(0),
  ATM_DELAY(4),
  ATM_VOL(58),
  ATM_NOTE_E2,
  ATM_DELAY(4),
  ATM_NOTE_E2,
  ATM_DELAY(4),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 14" q2  [12 bytes]
  ATM_VOL(0),
  ATM_DELAY(4),
  ATM_VOL(58),
  ATM_NOTE_E2,
  ATM_DELAY(4),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_NOTE_E2,
  ATM_DELAY(4),
  ATM_RETURN,

  //"Track 15" bs  [9 bytes]
  ATM_GOTO(13),   // -> q1
  ATM_GOTO(13),   // -> q1
  ATM_GOTO(14),   // -> q2
  ATM_GOTO(13),   // -> q1
  ATM_RETURN,

  //"Track 16" bI  [12 bytes]
  ATM_VOL(50),
  ATM_NOTE_E2,
  ATM_DELAY(64),
  ATM_SL_VOL(0),
  ATM_SET_TRA(0),
  ATM_GOTO(15),   // -> bs
  ATM_GOTO(17),   // -> bL

  //"Track 17" bL  [34 bytes]
  ATM_SET_TRA(0),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA(0),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA((uint8_t)-4),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA((uint8_t)-2),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA(0),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA(0),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA((uint8_t)-4),
  ATM_GOTO(15),   // -> bs
  ATM_SET_TRA(7),
  ATM_GOTO(15),   // -> bs
  ATM_GOTO(17),   // -> bL

  //"Track 18" hk  [11 bytes]
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_RETURN,

  //"Track 19" lI  [33 bytes]
  ATM_VOL(30),
  ATM_SL_VOL((uint8_t)-1),
  ATM_ARP(0x75, 0x20),
  ATM_NOTE_E3,
  ATM_DELAY(16),
  ATM_NOTE_G3,
  ATM_DELAY(16),
  ATM_NOTE_B3,
  ATM_DELAY(16),
  ATM_NOTE_E4,
  ATM_DELAY(16),
  ATM_NOTE_G4,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_GOTO(20),   // -> lL

  //"Track 20" lL  [10 bytes]
  ATM_GOTO(21),   // -> l1
  ATM_GOTO(22),   // -> l4
  ATM_GOTO(21),   // -> l1
  ATM_GOTO(23),   // -> l8
  ATM_GOTO(20),   // -> lL

  //"Track 21" l1  [39 bytes]
  ATM_GOTO(18),   // -> hk
  ATM_NOTE_E4,
  ATM_DELAY(12),
  ATM_NOTE_D4,
  ATM_DELAY(4),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_GOTO(18),   // -> hk
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_C3,
  ATM_DELAY(8),
  ATM_NOTE_C3,
  ATM_DELAY(4),
  ATM_NOTE_E3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_NOTE_C4,
  ATM_DELAY(12),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_G3,
  ATM_DELAY(8),
  ATM_NOTE_E3,
  ATM_DELAY(8),
  ATM_RETURN,

  //"Track 22" l4  [19 bytes]
  ATM_NOTE_D3,
  ATM_DELAY(8),
  ATM_NOTE_D3,
  ATM_DELAY(4),
  ATM_NOTE_F3_,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_F3_,
  ATM_DELAY(8),
  ATM_NOTE_D4,
  ATM_DELAY(12),
  ATM_NOTE_C4_,
  ATM_DELAY(4),
  ATM_NOTE_A3,
  ATM_DELAY(8),
  ATM_NOTE_F3_,
  ATM_DELAY(8),
  ATM_RETURN,

  //"Track 23" l8  [19 bytes]
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(4),
  ATM_NOTE_D4_,
  ATM_DELAY(4),
  ATM_NOTE_F4_,
  ATM_DELAY(8),
  ATM_NOTE_D4_,
  ATM_DELAY(8),
  ATM_NOTE_F4_,
  ATM_DELAY(8),
  ATM_NOTE_E4,
  ATM_DELAY(8),
  ATM_NOTE_D4_,
  ATM_DELAY(8),
  ATM_NOTE_B3,
  ATM_DELAY(8),
  ATM_RETURN,

};

#endif
