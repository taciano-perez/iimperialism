#ifndef SOUND_H
#define SOUND_H

/* Sound effect indices for play_sound() */
#define SOUND_TICK          0
#define SOUND_WHOPIDOOP     1
#define SOUND_PIP           2
#define SOUND_PHASOR        3
#define SOUND_MUSIC_SCALE   4
#define SOUND_SHORT_BRASS   5
#define SOUND_MEDIUM_BRASS  6
#define SOUND_LONG_BRASS    7
#define SOUND_GEIGER        8
#define SOUND_GLEEP         9
#define SOUND_GLISSADE      10
#define SOUND_QWIP          11
#define SOUND_OBOE          12
#define SOUND_FRENCH_HORN   13
#define SOUND_ENGLISH_HORN  14
#define SOUND_TIME_BOMB     15

void play_sound(unsigned char sound);

#endif
