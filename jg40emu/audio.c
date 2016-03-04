/*
	JG40 - Jaguar Virtual Video-Game Emulator
	version 1.0

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html
*/
#include "audio.h"

unsigned char volume = 32;
unsigned char tone_list[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
unsigned char play_tone = 0;
unsigned char last_play = 0;
unsigned char tone_count = 0;
unsigned char cur_tone = 0;
float cur_tone_freq = 0.0f;
float phase = 0.0f;
float timer = 0.0f;

void (*ToneFinished)() = 0;

/* table of tone's frequencies */
const float freq_table[16] = 
{
	261.63f, /* C  oct.4 */
	277.18f, /* C# */
	293.66f, /* D */
	311.13f, /* D# */
	329.63f, /* E */
	349.23f, /* F */
	369.99f, /* F# */
	392.00f, /* G */
	415.30f, /* G# */
	440.00f, /* A */
	466.16f, /* A# */
	493.88f, /* B */
	523.25f, /* C oct.5 */
	554.37f, /* C# */
	587.33f, /* D */
	622.25f  /* D# */
};


/* InitAudio
	Initialize SDL Audio and set the callback function to fill the sound buffer
*/
int InitAudio( void (*ToneFinishedCallback)() )
{	
	SDL_AudioSpec spec;

	/* setup the audio format */
    spec.freq = SOUND_FREQ;
    spec.format = AUDIO_S8;
    spec.channels = 0;
    spec.samples = SOUND_SAMPLES;
    spec.callback = FillAudio;
    spec.userdata = NULL;

	if( SDL_OpenAudio( &spec, NULL )<0 )
	{
		fprintf( stderr, "Couldn't open SDL Audio: <%s>\n", SDL_GetError() );
		return 0;
    }

	SDL_PauseAudio( 0 );

	ToneFinished = ToneFinishedCallback;

	return 1;
}


/* NextTone
	Move to the next tone in the sequence and detects its end
*/
int NextTone()
{
	/* reached the end? */
	if( cur_tone>=tone_count || play_tone==0 ) 
	{
		play_tone = 0;
		/* call the callback function when sequence finishes */
		if( ToneFinished ) ToneFinished();
		return 1;
	}

	/* move to the next tone */
	cur_tone_freq = SOUND_FREQ/freq_table[tone_list[cur_tone]];
	cur_tone++;

	return 0;
}


/* FillAudio
	Functions which fills the sound buffer
*/
void FillAudio( void *udata, Uint8 *stream, int len )
{
	Sint8 *ptr = (Sint8*)stream;
	Uint8 data;	
	int r;

	/* skip if no tones to play */
	if( play_tone==0 ) return;

	/* fill the waveform */
	for( r=0; r<len; r++ )
	{
		data = (Sint8)(volume*sin( PI*phase ));
		*ptr = data;
		ptr++;

		phase += 1/cur_tone_freq;
		if( phase>=2.0f ) phase -=2.0f;

		timer += TONE_STEP;
		if( timer>=1.0f )
		{
			timer = 0.0f;
			/* step to the next tone */
			if( NextTone()>0 ) break;
		}
	} 
}


/* StartPlay
	Initiate the playing of the programmed sequence of tones
*/
void StartPlay()
{
	play_tone = 1;
	cur_tone = 0;
	NextTone();
}


/* StopPlay
	Stop playing the sequence
*/
void StopPlay()
{
	if( play_tone!=0 )
	{
		last_play = cur_tone;

		play_tone = 0;
		cur_tone = 0;
	}
}


/* ContinuePlay
	Continue playing the sequence where it's stoped
*/
void ContinuePlay()
{
	if( last_play!=0 )
	{
		play_tone = 1;
		cur_tone = last_play;
		last_play = 0;
	}
}


/* IsPlaying
	Check if the sequence is playing
*/
int IsPlaying()
{
	return (play_tone!=0);
}


/* SetToneCount
	Set the number of tones in the sequence
*/
void SetToneCount( unsigned char count )
{
	tone_count = count;
}


/* SeTone
	Set one tone in the specified index of the sequence
*/
void SetTone( unsigned char index, unsigned char tone )
{
	tone_list[index] = tone;
}


/* IncreaseVolume
	Increase the tone sequence volume
*/
void IncreaseVolume()
{
	if( volume<127 ) 
		volume += 8;
	else
		volume = 127;
}


/* DecreaseVolume
	Decrease the tone sequence volume
*/
void DecreaseVolume()
{
	if( volume>8 ) 
		volume -= 8;
	else
		volume = 0;
}
