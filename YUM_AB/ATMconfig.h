#ifndef ATMCONFIG_H
#define ATMCONFIG_H

// The title tune (3 channels: lead, arpeggio chords, bass; transposition, tempo, octaves 2-4; no drums) and the sound effects (notes, volume). Everything else is left out.

// effects that are NOT used
#define ATM_FX_FREQ_SLIDE       0
#define ATM_FX_NOISE            0
#define ATM_FX_VIBRATO          0
#define ATM_FX_TREMOLO          0
#define ATM_FX_GLISSANDO        0
#define ATM_FX_NOTE_CUT         0
#define ATM_FX_CUE              0
#define ATM_FX_GOTO_ADV         0

// functions: ATM.playSfx() for the effects and ATM.stop() to switch the sound interrupt off afterwards (the maze needs the processor)
#define ATM_FUNC_PLAYPAUSE      0
#define ATM_FUNC_PAUSE          0
#define ATM_FUNC_RESUME         0
#define ATM_FUNC_MUTE           0
#define ATM_FUNC_UNMUTE         0
#define ATM_FUNC_TEMPO          0
#define ATM_FUNC_CUE            0

// the title tune uses channels 0-2 (pulse lead, square arpeggio, saw bass); channel 3 (noise) has no drums to play: out of the sound interrupt
#define ATM_WAVE_CH3            ATM_WAVE_OFF

// the title tune nests 3 calls deep (loop track -> bar -> beat): a call stack of 3 levels per channel (the default is 7)
#define ATM_STACK_DEPTH         3
#define ATM_OCTAVE_5            0
#define ATM_OCTAVE_6            0
#define ATM_OCTAVE_7            0

#endif
