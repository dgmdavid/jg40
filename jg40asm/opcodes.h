/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/

#include "global.h"


/* keywords */
#define NUM_KEYWORDS  5 

extern char *Keywords[NUM_KEYWORDS];


/* mnemonics/opcodes */
#define NUM_OPCODES  16

extern struct SOpcode
{
	char *mnemonic;
	byte opcode;
	byte size;
} Opcodes[NUM_OPCODES];
