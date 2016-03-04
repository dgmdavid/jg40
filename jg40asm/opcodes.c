/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/

#include "opcodes.h"


char *Keywords[NUM_KEYWORDS] =
{
	"INCLUDE", "INCBIN", "ORG", "DN", "DB"
};


struct SOpcode Opcodes[NUM_OPCODES] =
{ 
/* mnemonic, opcode, size (in nibbles, including argument) */
   "LDA", 0x0, 1,
   "STA", 0x1, 1,
   "ADD", 0x2, 1,
   "OR",  0x3, 2,
   "AND", 0x4, 2,
   "NOT", 0x5, 1, 
   "SHL", 0x6, 1, 
   "SHR", 0x7, 1, 
   "PUA", 0x8, 1, 
   "POA", 0x9, 1, 
   "CMP", 0xA, 1, 
   "JZ",  0xB, 4,
   "JC",  0xC, 4,
   "SED", 0xD, 2,
   "SES", 0xE, 4,
   "TRR", 0xF, 1
};
