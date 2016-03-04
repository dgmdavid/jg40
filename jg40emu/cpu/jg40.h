/*
	JG40 - Jaguar Virtual Video-Game
	
	JG40 CPU emulator

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html
*/

#ifndef __JG40_CPU_H_
#define __JG40_CPU_H_

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned int uint;

/* memory sizes, in nibbles */
#define RAM_SIZE     16
#define SRAM_SIZE    16
#define VRAM_SIZE    64  /* vram and sndram will store each nibble in  */
#define SNDRAM_SIZE  16  /* a byte, in order to make life easier  */
#define ROM_SIZE   4096
#define MEMORY_MAPPED_IO_ADDRESS 0xF

#define FLAG_ZERO   1
#define FLAG_CARRY  2

#define TRUE    1
#define FALSE  -1


struct JG40_CPU
{
	byte RAM[RAM_SIZE>>1];
	byte SRAM[SRAM_SIZE>>1];
	byte VRAM[VRAM_SIZE];     /* "unpacked" */
	byte SNDRAM[SNDRAM_SIZE]; /* "unpacked" */
	byte ROM[ROM_SIZE>>1];

	byte accumulator;     /* A - 4-bit Accumulator */
	byte RAM_pointer;     /* D - 4-bit RAM Data Address pointer */
	byte SRAM_pointer;    /* T - 4-bit Stack Pointer */
	/* TODO: drop VRAM_pointer */
	byte VRAM_pointer;    /* ? - 6-bit Video Pointer */
	word ROM_pointer;     /* S - 12-bit ROM Data Address pointer */
	word program_pointer; /* P - 12-bit Program Pointer */ 

	byte zero_flag;
	byte carry_flag;

	word loaded_ROM_size;
	byte finished;
} jg40;


/* pointers to the Memory Mapped I/O callbacks */
extern byte (*MMIOCB_write)(byte);
extern byte (*MMIOCB_read)();


/* opcodes */
#define LDA  0x00
#define STA  0x01
#define ADD  0x02
#define OR   0x03
#define AND  0x04
#define NOT  0x05
#define SHL  0x06
#define SHR  0x07
#define PUA  0x08
#define POA  0x09
#define CMP  0x0A
#define JZ   0x0B
#define JC   0x0C
#define SED  0x0D
#define SES  0x0E
#define TRR  0x0F


/* cycles taken by each opcode */
extern byte OPCODE_CYCLES[16];
/* names of the opcodes */
extern char OPCODE_NAMES[16][4];

extern int cycle_count;

void jg40_initialize();
int jg40_loadROM( byte *ROM, int size );
void jg40_loadstate( struct JG40_CPU *state );
void jg40_reset();
int jg40_execute( int cycles );
void jg40_setmemorymappedIOcallbacks( byte (*writecallback)(byte), byte (*readcallback)() );

byte jg40_read4bit( byte *MEM, word address );
byte jg40_read8bit( byte *MEM, word address );
word jg40_read12bit( byte *MEM, word address );

byte jg40_write4bit( byte *MEM, word address, byte value );
void jg40_write8bit( byte *MEM, word address, byte value );
void jg40_write12bit( byte *MEM, word address, word value );

void jg40_takecycles( int cycles );

void jg40_getinstruction( word *address, char *inst );

#endif