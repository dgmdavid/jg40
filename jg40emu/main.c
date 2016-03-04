/*
	JG40 - Jaguar Virtual Video-Game Emulator
	version 1.0

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html

	This is the Jaguar 4-bit (JG40) emulator.

	Rev 0.1 29/10/2007 -- Begin of work.
*/
#include <stdio.h>

#include <math.h>
#include "SDL.h"
#pragma comment (lib, "sdl.lib")
#pragma comment (lib, "sdlmain.lib")

#include "cpu/jg40.h"
#include "defines.h"
#include "audio.h"

SDL_Event event;
SDL_Surface *screen;
SDL_Surface *console;
SDL_Surface *font[4];
SDL_Rect dest, dest2;
char text_buffer[256];

/* the current mode of the memory mapped I/O port */
int mode; 
/* auxiliary variables */
int aux, aux2, aux3, aux4; 

byte CPU_state, CPU_previous;
byte ROM_loaded;
byte ROM[ROM_SIZE];
byte running_speed; /* 0 to 12 - 3 = 100%  */
byte sound_enabled;

/* running speeds - (14800/20) = 740 cycles = 100% */
int speeds[] =  { 2220, 1480, 1110, 740, 666, 592, 518, 444, 370, 296, 222, 148, 74, 37 };
int speedsp[] = {  300,  200,  150, 100,  90,  80,  70,  60,  50,  40,  30,  20, 10,  5 };

/* buttons state */
byte buttons = 0;


/* LoadFont
	Load the bitmap font and make four colors of it 
*/
int LoadFont( const char *file_name )
{
    /* load font file */
	font[0] = SDL_LoadBMP( file_name );
    if( font[0]==NULL )
	{
        fprintf( stderr, "Unable to load font \"%s\": %s\n", file_name, SDL_GetError() );
		return -1;
	}

	/* create duplicates with other colors */
	font[FONT_RED] = SDL_CreateRGBSurface( SDL_SWSURFACE, font[0]->w, font[0]->h, 8, 255,   0,   0, 0 );
	font[FONT_GREEN] = SDL_CreateRGBSurface( SDL_SWSURFACE, font[0]->w, font[0]->h, 8, 0,   255,   0, 0 );
	font[FONT_BLUE] = SDL_CreateRGBSurface( SDL_SWSURFACE, font[0]->w, font[0]->h, 8, 0,     0, 255, 0 );

	SDL_BlitSurface( font[0], NULL, font[1], NULL );
	SDL_BlitSurface( font[0], NULL, font[2], NULL );
	SDL_BlitSurface( font[0], NULL, font[3], NULL );

	SDL_SetColorKey( font[0], SDL_SRCCOLORKEY, 1 );
	SDL_SetColorKey( font[1], SDL_SRCCOLORKEY, 0 );
	SDL_SetColorKey( font[2], SDL_SRCCOLORKEY, 0 );
	SDL_SetColorKey( font[3], SDL_SRCCOLORKEY, 0 );
	
	return 0;
}


/* WriteText
	Write text with bitmap font 
*/
void WriteText( int x, int y, byte color, const char *text, ... )
{
    int i;
    byte l;
	SDL_Rect dest;
	SDL_Rect src;

    va_list ap;
    va_start( ap, text );
    vsprintf( text_buffer, text, ap );
    va_end( ap );

	dest.x = x;
	dest.y = y;
	src.w = 8;
	src.h = 8;

    for( i=0; text_buffer[i]!='\0'; ++i )
	{
		l = (byte)text_buffer[i]; 
		if( l!=32 )
		{
			src.x = (l%16)*8;
			src.y = (l/16)*8;
			SDL_BlitSurface( font[color], &src, screen, &dest );
		}
		dest.x += 8;
	}
}


/* DrawConsole
	Draw the jg40 console image on the screen
*/
void DrawConsole()
{
	SDL_Rect rect;
	rect.x = 3;
	rect.y = 25;
	rect.w = console->w;
	rect.h = console->h;
	SDL_BlitSurface( console, NULL, screen, &rect );
}


/* DrawDumps
	Draw the dump values of jg40 registers, RAM and stack	
*/
void DrawDumps()
{
	SDL_Rect rect;

	/* draw registers */
	rect.x = 1;
	rect.y = 1;
	rect.w = 80; 
	rect.h = 56;
	SDL_FillRect( screen, &rect, 0 );
	WriteText( 1,  1, FONT_RED,   "Registers:" );
	WriteText( 1,  9, FONT_WHITE, "A:  %01X", jg40.accumulator );
	WriteText( 1, 17, FONT_WHITE, "D:  %01X", jg40.RAM_pointer );
	WriteText( 1, 25, FONT_WHITE, "S:%03X",   jg40.ROM_pointer );
	WriteText( 1, 33, FONT_WHITE, "P:%03X",   jg40.program_pointer );
	WriteText( 1, 41, FONT_WHITE, "T:  %01X", jg40.SRAM_pointer );
	WriteText( 1, 49, FONT_WHITE, "F: %01X%01X", jg40.zero_flag, jg40.carry_flag );

	/* draw RAM dump */
	rect.x = 384;
	rect.y = 1;
	rect.w = 257; 
	rect.h = 16;
	SDL_FillRect( screen, &rect, 0 );
	WriteText( 385, 1, FONT_RED, "RAM:" );

	/* draw RAM pointer */
	rect.x = 384+(jg40.RAM_pointer*8);
	rect.y = 8;
	rect.w = 9; 
	rect.h = 9;
	SDL_FillRect( screen, &rect, 6 );
	WriteText( 385, 9, FONT_WHITE, "%02X%02X%02X%02X%02X%02X%02X%02X%", jg40.RAM[0], jg40.RAM[1], jg40.RAM[2], jg40.RAM[3], jg40.RAM[4], jg40.RAM[5], jg40.RAM[6], jg40.RAM[7] );

	/* draw Stack dump */
	rect.x = 191;
	rect.y = 1;
	rect.w = 129; 
	rect.h = 16;
	SDL_FillRect( screen, &rect, 0 );
	WriteText( 192, 1, FONT_RED, "Stack:" );

	/* draw Stack pointer */
	rect.x = 191+(jg40.SRAM_pointer*8);
	rect.y = 8;
	rect.w = 9; 
	rect.h = 9;
	SDL_FillRect( screen, &rect, 6 );
	WriteText( 192, 9, FONT_WHITE, "%02X%02X%02X%02X%02X%02X%02X%02X", jg40.SRAM[0], jg40.SRAM[1],	jg40.SRAM[2], jg40.SRAM[3], jg40.SRAM[4], jg40.SRAM[5],	jg40.SRAM[6], jg40.SRAM[7] );
}


/* DrawDisassembler
	Draw the disassembled code of the current portion of ROM
*/
void DrawDisassembler()
{
	int r;
	byte color;
	char inst[10];
	word addr;
	SDL_Rect rect;

	WriteText( 336, 31, FONT_RED, "Disassembler:" );
	
	rect.x = 336;
	rect.y = 31+8;
	rect.w = (22*8);
	rect.h = (22*8);
	SDL_FillRect( screen, &rect, 224 );
	
	if( ROM_loaded )
	{
		addr = jg40.program_pointer;
		for( r=0; r<22; r++ )
		{
			color = FONT_GREEN;
			if( addr==jg40.program_pointer ) color = FONT_WHITE;

			if( addr>=jg40.loaded_ROM_size ) break;

			WriteText( 344, 39+(r*8), color, "0x%03X:", addr ); 
			jg40_getinstruction( &addr, inst );
			WriteText( 400, 39+(r*8), color, "%s", inst ); 
		}
	}
}


/* DrawBottom
	Draw the bottom part of screen, with the shortcut keys
*/
void DrawBottom()
{
	SDL_Rect rect;
	
	rect.x = 0;
	rect.y = 220;
	rect.w = 513;
	rect.h = 9;
	SDL_FillRect( screen, &rect, 7 );

	WriteText( 1, 221, FONT_BLUE, "F1-Load  F2-Run  F3-Reset  F4-Pause  F5-Step            F10-Quit" );
	
	if( !ROM_loaded ) return;
	if( CPU_state==CPU_PAUSED )
	{
		WriteText( 217, 221, FONT_RED, "F4-Pause" );
	} else {
		WriteText( 73, 221, FONT_RED, "F2-Run" );
	}
}


/* DrawLCDScreen
	Draw the contents of the jg40 LCD screen display
*/
void DrawLCDScreen()
{
	byte x, y, aux, color;

	dest.w = 5;
	dest.h = 5;

	aux = 0;
	for( y=0; y<16; y++ )
	{
		dest.y = 69+(y*6);
		for( x=0; x<4; x++ )
		{
			if( (jg40.VRAM[aux]&8) ) color = 0; else color = 20;
			dest.x=121+(x*6*4);SDL_FillRect( screen, &dest, color );

			if( (jg40.VRAM[aux]&4) ) color = 0; else color = 20;
			dest.x=127+(x*6*4);SDL_FillRect( screen, &dest, color );

			if( (jg40.VRAM[aux]&2) ) color = 0; else color = 20;
			dest.x=133+(x*6*4);SDL_FillRect( screen, &dest, color );

			if( (jg40.VRAM[aux]&1) ) color = 0; else  color = 20;
			dest.x=139+(x*6*4);SDL_FillRect( screen, &dest, color );

			aux++;
		}
	}

	/* resume the CPU */
	if( mode==MODE_WAIT_VSYNC )
	{
		CPU_state = CPU_previous;
		mode = MODE_IDLE;
	}
}


/* MMIOPortWrite
	Callback function to handle memory mapped I/O port writes 
*/
byte MMIOPortWrite( byte value )
{
	/* no action being performed? */
	if( mode==MODE_IDLE )
	{
		aux = 0;
		aux2 = 0;
		aux3 = 0;
		aux4 = 0;
		mode = value;
	
	}

/* Video part ************************************************************/

	/* $0 - unset a pixel ************************************************/
	if( mode==MODE_UNSET_PIXEL )
	{
		aux++;
		if( aux==2 ) aux2 = value; /* get X position */
		if( aux==3 )
		{
			aux3 = value; /* get Y position */
			
			/* find byte and mask to operate on VRAM */
			aux4 = (aux3*4)+(aux2/4);
			aux2 = 8>>(aux2%4);
			jg40.VRAM[aux4] &= ~aux2;

			jg40_takecycles( 4 );
			mode = MODE_IDLE;
		}
	} else

	/* $1 - set a pixel **************************************************/
	if( mode==MODE_SET_PIXEL )
	{
		aux++;
		if( aux==2 ) aux2 = value; /* get the X position */
		if( aux==3 )
		{
			aux3 = value; /* get the Y position */

			/* find byte and mask to operate on VRAM */
			aux4 = (aux3*4)+(aux2/4);
			aux2 = 8>>(aux2%4);
			jg40.VRAM[aux4] |= aux2;

			jg40_takecycles( 4 );
			mode = MODE_IDLE;
		}
	} else

	/* $2 - check if a pixel is set **************************************/
	if( mode==MODE_CHECK_PIXEL )
	{
		aux++;
		if( aux==2 ) aux2 = value; /* get the X position */
		if( aux==3 ) 
		{
			aux3 = value; /* get the Y position */
			/* will wait for a read */
		}
	} else

	/* $3 - fill the entire screen with 64 successive writes *************/
	if( mode==MODE_FILL_SCREEN )
	{
		if( aux>0 )	jg40.VRAM[aux-1] = value;
		aux++;
		jg40_takecycles( 2 );
		if( aux==65 ) mode = MODE_IDLE;
	} else

	/* $4 - wait for vsync ***********************************************/
	if( mode==MODE_WAIT_VSYNC )
	{
		CPU_previous = CPU_state;
		CPU_state = CPU_WAITING;
		return 1;
	} else

	/* $5 - clear the screen *********************************************/
	if( mode==MODE_CLEAR_SCREEN )
	{
		for( aux=0; aux<64; aux++ ) jg40.VRAM[aux] = 0;
		jg40_takecycles( 64 );
		mode = MODE_IDLE;
	}

	/* $6 - perform screen scrolling *************************************/
	if( mode==MODE_SCROLL_SCREEN )
	{
		aux++;
		if( aux==2 ) 
		{
			aux2 = value; 

			/* left-to-right scrolling */
			if( aux2<=3 )
			{
				byte mask, pos;
				if( aux2==0 ) mask = 1; else
				if( aux2==1 ) mask = 3; else
				if( aux2==2 ) mask = 7; else
				if( aux2==3 ) mask = 15; 
				for( aux3=0; aux3<16; aux3++ )
				{
					for( aux4=3; aux4>=0; aux4-- )
					{
						pos = (aux3*4)+aux4;

						jg40.VRAM[pos] = jg40.VRAM[pos]>>(aux2+1);
						if( aux4>0 ) jg40.VRAM[pos] |= (jg40.VRAM[pos-1]&mask)<<(3-aux2);
					}
				}
			} else

			/* right-to-left scrolling */
			if( aux2<=7 )
			{
				byte mask, pos;
				aux2 -= 4;
				if( aux2==0 ) mask = 8; else
				if( aux2==1 ) mask = 12; else
				if( aux2==2 ) mask = 14; else
				if( aux2==3 ) mask = 15; 
				for( aux3=0; aux3<16; aux3++ )
				{
					for( aux4=0; aux4<=3; aux4++ )
					{
						pos = (aux3*4)+aux4;
						jg40.VRAM[pos] = jg40.VRAM[pos]<<(aux2+1);
						if( aux4<3 ) jg40.VRAM[pos] |= (jg40.VRAM[pos+1]&mask)>>(3-aux2);
					}
				}
			} else

			/* top-to-bottom scrolling */
			if( aux2<=11 )
			{
				int pos;
				aux2 -= 8;
				for( aux3=15; aux3>=aux2+1; aux3-- )
				{
					pos = (aux3*4);
					jg40.VRAM[pos] = jg40.VRAM[pos-(4+aux2*4)];
					jg40.VRAM[pos+1] = jg40.VRAM[pos-(3+aux2*4)];
					jg40.VRAM[pos+2] = jg40.VRAM[pos-(2+aux2*4)];
					jg40.VRAM[pos+3] = jg40.VRAM[pos-(1+aux2*4)];
				}
				for( aux3=0; aux3<aux2+1; aux3++ )
				{
					pos = (aux3*4);
					jg40.VRAM[pos] = 0;
					jg40.VRAM[pos+1] = 0;
					jg40.VRAM[pos+2] = 0;
					jg40.VRAM[pos+3] = 0;
				}
			} else

			/* bottom-to-top scrolling */
			{
				int pos;
				aux2 -= 11;
				for( aux3=0; aux3<=15-aux2; aux3++ )
				{
					pos = (aux3*4);
					jg40.VRAM[pos] = jg40.VRAM[pos+(aux2*4)];
					jg40.VRAM[pos+1] = jg40.VRAM[pos+(1+aux2*4)];
					jg40.VRAM[pos+2] = jg40.VRAM[pos+(2+aux2*4)];
					jg40.VRAM[pos+3] = jg40.VRAM[pos+(3+aux2*4)];
				}
				for( aux3=15; aux3>=15-(aux2-1); aux3-- )
				{
					pos = (aux3*4);
					jg40.VRAM[pos] = 0;
					jg40.VRAM[pos+1] = 0;
					jg40.VRAM[pos+2] = 0;
					jg40.VRAM[pos+3] = 0;
				}
			} 

			jg40_takecycles( 256 );
			mode = MODE_IDLE;
		}
	}

	/* $7 - negate the screen ********************************************/
	if( mode==MODE_NEGATE_SCREEN )
	{
		for( aux=0; aux<64; aux++ ) jg40.VRAM[aux] = ~jg40.VRAM[aux];
		jg40_takecycles( 64 );
		mode = MODE_IDLE;
	}

/* Sound part ************************************************************/

	/* $A - program a sequence of tones **********************************/
	if( mode==MODE_PROGRAM_TONE )
	{
		aux++;
		if( aux<=2 )
		{
			aux2 = value+1;
			jg40_takecycles( 2 );
		} else {
			jg40.SNDRAM[aux-3] = value;
			jg40_takecycles( 2 );
			if( aux==aux2+2 ) 
			{
				if( !IsPlaying() )
				{
					SetToneCount( (unsigned char)aux2 );
					for( aux=0; aux<16; aux++ )	SetTone( (unsigned char)aux, jg40.SNDRAM[aux] );
				}
				mode = MODE_IDLE;
			}
		}
	} else

	/* $B - play/replay the last programmed sequence *********************/
	if( mode==MODE_PLAY_TONE )
	{
		if( !IsPlaying() )
		{
			if( sound_enabled ) StartPlay();
			jg40_takecycles( 4 );
		}
		mode = MODE_IDLE;
	} else

	/* $C - check if a sequence is playing *******************************/
	if( mode==MODE_CHECK_TONE )
	{
		/* will wait for a read */
	} else

	/* $D - stop the current music ***************************************/
	if( mode==MODE_STOP_TONE )
	{
		if( sound_enabled ) StopPlay();
		jg40_takecycles( 4 );
		mode = MODE_IDLE; 
	} else

	/* $E - wait sequence of tones to finish *****************************/
	if( mode==MODE_WAIT_TONE )
	{
		if( sound_enabled )
		{
			CPU_previous = CPU_state;
			CPU_state = CPU_WAITING;
		}
		return 1;
	}

/* Input part ************************************************************/

	/* $F - get buttons state ********************************************/
	if( mode==MODE_CHECK_BUTTONS )
	{
		/* will wait for a read */
	}

	return 0;
}



/* MMIOPortRead
	Callback function to handle memory mapper I/O port reads 
*/
byte MMIOPortRead()
{
	/* $2 - check if a pixel is set **************************************/
	if( mode==MODE_CHECK_PIXEL )
	{
			/* find byte and mask to operate on VRAM */
			aux4 = (aux3*4)+(aux2/4);
			aux2 = 8>>(aux2%4);
			jg40_takecycles( 5 );
			mode = MODE_IDLE;
			return (jg40.VRAM[aux4]&aux2)>0;
	} else
	
	/* $C - check if a sequence is playing *******************************/
	if( mode==MODE_CHECK_TONE )
	{
		jg40_takecycles( 4 );
		mode = MODE_IDLE;
		return IsPlaying();
	} else

	/* $F - return the buttons state *************************************/
	if( mode==MODE_CHECK_BUTTONS )
	{
		jg40_takecycles( 4 );
		mode = MODE_IDLE;
		return buttons; 
	}

	return 0;
}


/* FinishedTone
	This function will be called when a sequence of tones finishes playing
*/
void FinishedTone()
{
	/* resume the CPU */
	if( mode==MODE_WAIT_TONE )
	{
		//if( CPU_previous==0 ) CPU_paused = 0;
		CPU_state = CPU_previous;
		mode = MODE_IDLE;
	}
}


/* LoadROM
	Load a file that contains jg40 binary code (ROM)
*/
int LoadROM( const char *file_name )
{
	FILE *f;
	int file_size;
	
	if( file_name[0]=='\0' ) return 0;

	ROM_loaded = 0;
	jg40_initialize();
	
	f = fopen( file_name, "rb" );
	if( f==NULL )
	{
	    fprintf( stderr, "Couldn't load ROM file \"%s\".\n", file_name );
		return 0;
	}
	
	fseek( f, 0, SEEK_END );
	if( ftell( f )>(ROM_SIZE>>1) )
	{
		fprintf( stderr, "ROM file \"%s\" exceeds the maximum ROM size.\n", file_name );
		fclose( f );
		return 0;
	}
	fseek( f, 0, SEEK_SET );

	file_size = fread( &ROM, 1, ROM_SIZE, f );
	fclose( f );
	
	jg40_loadROM( ROM, file_size );
	ROM_loaded = 1;
	printf( "ROM file \"%s\" loaded successfully.\n", file_name );

	return 1;
}


/* UpdateCaption
	Update the window title
*/
void UpdateCaption()
{
	if( !ROM_loaded )
	{
		SDL_WM_SetCaption( CAPTION_NORMAL, NULL );
	} else {
		if( CPU_state==CPU_PAUSED )
		{
			SDL_WM_SetCaption( CAPTION_PAUSED, NULL );
		} else
		{
			char temp[31];
			sprintf( temp, "%s: %d%%", CAPTION_RUNNING, speedsp[running_speed] );
			SDL_WM_SetCaption( temp, NULL );
		}
	}
}


/* Pause */
void Pause()
{
	if( CPU_state!=CPU_PAUSED )
	{
		SDL_WM_SetCaption( CAPTION_PAUSED, NULL );
	}
	if( CPU_state!=CPU_WAITING )
		CPU_state = CPU_PAUSED;
	else
		CPU_previous = CPU_PAUSED;
	StopPlay();
	UpdateCaption();
}


/*
	Main function
*/
int main( int argc, char *argv[] ) 
{
	int exit_;
	Uint32 time1, time2, wait;
	int odd;
	char letter;
	char filename[69];
	char filename_pos;

	/* TODO: make sure the code is 100% ANSI-compliant */

	/* initialize variables */
	CPU_state = CPU_PAUSED;
	CPU_previous = CPU_RUNNING;
	running_speed = 3;
	mode = MODE_IDLE;
	aux = 0;

	ROM_loaded = 0;
	odd = 0;
	filename[0] = '\0';
	filename_pos = -1;

	/* welcome the user */
	printf( "JG40 Emulator v1.0\n" );
	printf( "Developed by David G. Maziero (http://br.geocities.com/dgmsofthp/jaguar.html)\n\n");

    /* initializes SDL video */
	printf( "Initializing SDL: \n" );
	if( SDL_Init( SDL_INIT_VIDEO )==-1 )
	{ 
        fprintf(stderr, "Couldn't initialize SDL: <%s>\n", SDL_GetError() );
        exit( -1 );
    }

    /* initializes SDL audio */
	sound_enabled = 1;
	if( SDL_Init( SDL_INIT_AUDIO )==-1 )
	{ 
        fprintf(stderr, "Couldn't initialize SDL: <%s>\n", SDL_GetError() );
        sound_enabled = 0;
    }

    screen = SDL_SetVideoMode( 513, 229, 8, SDL_SWSURFACE );
    if( screen==NULL ) 
	{
        fprintf(stderr, "Error initializing SDL Video: <%s>\n", SDL_GetError() );
		exit( -1 );
	}

	SDL_WM_SetCaption( CAPTION_NORMAL, NULL );

	LoadFont( "font.bmp" );

    /* load the "console" bitmap */
    console = SDL_LoadBMP( "console.bmp" );
    if( console==NULL )
	{
        fprintf(stderr, "Couldn't load \"console.bmp\": <%s>\n", SDL_GetError() );
		exit( -1 );
    }

    SDL_SetColors( screen, console->format->palette->colors, 0, console->format->palette->ncolors );
	console = SDL_DisplayFormat( console );

	if( sound_enabled ) InitAudio( FinishedTone );
	
	printf( "Initializing JG40 CPU.\n" );
	jg40_initialize();
	jg40_setmemorymappedIOcallbacks( MMIOPortWrite, MMIOPortRead );

	/* load the file passed as an argument, if any */
	if( argc>1 )
	{
		if( !LoadROM( argv[1] ) ) exit( -1 );
	}

	/* draw all the fixed screen stuff */
	SDL_FillRect( screen, NULL, 0 );
	DrawConsole();
	DrawBottom();

	exit_ = 1;
	while( exit_ ) 
	{
		time1 = SDL_GetTicks();

		DrawDumps();
		DrawDisassembler();
		DrawBottom();

		/* since the emulator runs at 20 frames per second, and the jg40
		   lcd screen updates every 100ms, we call it on every odd time */
		odd=!odd;
		if( odd ) DrawLCDScreen();
	
		/* get SDL events */
		while( SDL_PollEvent(&event) )
		{
			switch( event.type )
			{
				case SDL_KEYDOWN:

					/* it will take care of the file name input on F1-Load */
					if( filename_pos>=0 )
					{
						letter = (char)event.key.keysym.sym;
						if( letter>=32 && letter<=122 && filename_pos<69 )
						{
							filename[filename_pos] = letter;
							filename_pos++;
							filename[filename_pos] = '\0';
						} else
						/* backspace */
						if( letter==SDLK_BACKSPACE && filename_pos>0 ) 
						{
							filename_pos--;
							filename[filename_pos]='\0';
						} else
						/* enter */
						if( letter==SDLK_RETURN )
						{
							LoadROM( filename );
							CPU_state = CPU_PAUSED;
							filename_pos = -1;
							dest.x = 0;
							dest.y = 211;
							dest.w = 513;
							dest.h = 9;
							SDL_FillRect( screen, &dest, 0 );
							UpdateCaption();
						}
					} else {
					
						/* buttons down */
						if( event.key.keysym.sym==SDLK_a || event.key.keysym.sym==SDLK_LEFT  ) buttons |= 8; else
						if( event.key.keysym.sym==SDLK_s || event.key.keysym.sym==SDLK_UP    ) buttons |= 4; else
						if( event.key.keysym.sym==SDLK_d || event.key.keysym.sym==SDLK_DOWN  ) buttons |= 2; else
						if( event.key.keysym.sym==SDLK_f || event.key.keysym.sym==SDLK_RIGHT ) buttons |= 1;
					}
					break;

				case SDL_KEYUP:
				  
					/* if in file name input mode, do not check other keys */
					if( filename_pos>=0 ) break;

					/* buttons up */
					if( event.key.keysym.sym==SDLK_a || event.key.keysym.sym==SDLK_LEFT  ) buttons &= ~8; else
					if( event.key.keysym.sym==SDLK_s || event.key.keysym.sym==SDLK_UP    ) buttons &= ~4; else
					if( event.key.keysym.sym==SDLK_d || event.key.keysym.sym==SDLK_DOWN  ) buttons &= ~2; else
					if( event.key.keysym.sym==SDLK_f || event.key.keysym.sym==SDLK_RIGHT ) buttons &= ~1;

					/* running speed change */
					if( event.key.keysym.sym==SDLK_ASTERISK || event.key.keysym.sym==SDLK_KP_MULTIPLY )
					{
						if( running_speed>0 ) running_speed--;
						UpdateCaption();
					} else

					if( event.key.keysym.sym==SDLK_SLASH || event.key.keysym.sym==SDLK_KP_DIVIDE )
					{
						if( running_speed<13 ) running_speed++;
						UpdateCaption();
					} else

					/* sound volume change */
					if( event.key.keysym.sym==SDLK_PLUS || event.key.keysym.sym==SDLK_KP_PLUS )
					{
						IncreaseVolume();
					} else

					if( event.key.keysym.sym==SDLK_MINUS || event.key.keysym.sym==SDLK_KP_MINUS )
					{
						DecreaseVolume();
					} else

					/* F1-Load */
					if( event.key.keysym.sym==SDLK_F1 )
					{
						Pause();
						filename_pos = 0;
					} else

					/* F2-Run */
					if( event.key.keysym.sym==SDLK_F2 )
					{
						if( CPU_state!=CPU_RUNNING )
						{
							ContinuePlay();
						}
						if( CPU_state!=CPU_WAITING ) CPU_state = CPU_RUNNING;
						UpdateCaption();
					} else

					/* F3-Reset */
					if( event.key.keysym.sym==SDLK_F3 )
					{
						mode = MODE_IDLE;
						if( CPU_state==CPU_WAITING )
						{
							CPU_state = CPU_RUNNING;
							CPU_previous = CPU_PAUSED;
						}
						ContinuePlay();
						StopPlay();
						jg40_reset();
					} else

					/* F4-Pause */
					if( event.key.keysym.sym==SDLK_F4 )
					{
						Pause();
					} else

					/* F5-Step */
					if( event.key.keysym.sym==SDLK_F5 )
					{
						Pause();
						if( ROM_loaded ) jg40_execute( 1 );
					} else

					/* F10-Quit */
					if( event.key.keysym.sym==SDLK_F10 )
					{
						exit_ = 0;
					}

					break;
			    default:;
			}
		}

		/* show the file name that is being inputted */
		if( filename_pos>=0 )
		{
			dest.x = 0;
			dest.y = 211;
			dest.w = 513;
			dest.h = 9;
			SDL_FillRect( screen, &dest, 7 );
			WriteText( 1, 212, FONT_BLUE, "File: %s", filename );
		}

		/* visual feedback for buttons state */
		{
			byte colors[4] = { FONT_RED, FONT_RED, FONT_RED, FONT_RED };
			if( (buttons&8)==8 ) colors[0] = FONT_GREEN;
			if( (buttons&4)==4 ) colors[1] = FONT_GREEN;
			if( (buttons&2)==2 ) colors[2] = FONT_GREEN;
			if( (buttons&1)==1 ) colors[3] = FONT_GREEN;
			WriteText( 57,120, colors[0], "*" );
			WriteText( 88,150, colors[1], "*" );
			WriteText( 242,150, colors[2], "*" );
			WriteText( 273,120, colors[3], "*" );
		}

		/* execute the jg40 code */
		if( CPU_state==CPU_RUNNING && ROM_loaded ) jg40_execute( speeds[running_speed] ); //(14800/20)

		SDL_UpdateRect( screen, 0, 0, 0, 0 );
	
		/* limite the frame rate to 20 */
		time2 = SDL_GetTicks()-time1;
		if( time2==0 ) time2 = 1;
		wait = 50-time2;
		if( wait>0 && wait<1000 ) SDL_Delay( wait );
	}

	/* free allocated resources */
	SDL_FreeSurface( console );
	SDL_FreeSurface( font[0] );
	SDL_FreeSurface( font[1] );
	SDL_FreeSurface( font[2] );
	SDL_FreeSurface( font[3] );
	SDL_FreeSurface( screen );

    SDL_Quit();
    
	return 0;
}

