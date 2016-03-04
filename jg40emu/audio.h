/*
	JG40 - Jaguar Virtual Video-Game Emulator
	version 1.0

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html
*/
#ifndef __AUDIO__
#define __AUDIO__

#include "SDL.h"
#include "math.h"

/* constants */
#define PI			   3.14159f
#define SOUND_FREQ	       8000
#define SOUND_SAMPLES       200
#define TONE_STEP	   0.00025f // 1/(8000/2) will produce a ~250ms tone


int InitAudio( void (*ToneFinishedCallback)() );
void FillAudio( void *udata, Uint8 *stream, int len );
int NextTone();

void StartPlay();
void StopPlay();
void ContinuePlay();
int IsPlaying();
void SetToneCount( unsigned char count );
void SetTone( unsigned char index, unsigned char tone );
void IncreaseVolume();
void DecreaseVolume();

#endif
