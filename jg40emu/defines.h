/*
	JG40 - Jaguar Virtual Video-Game Emulator
	version 1.0

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html
*/
#ifndef __DEFINES__
#define __DEFINES__

/* font colours */
#define FONT_WHITE	0
#define FONT_RED	1
#define FONT_GREEN	2
#define FONT_BLUE	3

/* cpu states */
#define CPU_RUNNING		0
#define CPU_PAUSED		1
#define CPU_WAITING		2

/* modes for the MMIO port */
#define MODE_IDLE            -1
#define MODE_UNSET_PIXEL    0x0
#define MODE_SET_PIXEL      0x1
#define MODE_CHECK_PIXEL    0x2
#define MODE_FILL_SCREEN    0x3
#define MODE_WAIT_VSYNC     0x4
#define MODE_CLEAR_SCREEN   0x5
#define MODE_SCROLL_SCREEN  0x6
#define MODE_NEGATE_SCREEN  0x7
#define MODE_PROGRAM_TONE   0xA
#define MODE_PLAY_TONE      0xB
#define MODE_CHECK_TONE     0xC
#define MODE_STOP_TONE      0xD
#define MODE_WAIT_TONE      0xE
#define MODE_CHECK_BUTTONS  0xF

#define CAPTION_NORMAL  "JG40 Emulator"
#define CAPTION_RUNNING "JG40 Emulator - Running"
#define CAPTION_PAUSED  "JG40 Emulator - Paused"

#endif
