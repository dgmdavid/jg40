/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Basic types */
typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned int uint;

/* Constant and defines */
#define START_ADDRESS     0 /* program entry-point */
#define MAX_LINE_SIZE   256
#define MAX_SYMBOLS    1024
#define MAX_FILES	      8 /* no more than 8 files included */
#define MAX_VALUE      4096 /* the max. value is 12-bit long */
#define MAX_ROM_BSIZE  2048 /* max ROM size in bytes */
#define MAX_ROM_NSIZE  4096 /* max ROM size in nibbles */

#define version_major     1
#define version_minor     0

#define FALSE     0
#define TRUE	  1
#define ERROR	 -1

#define Hex_Notation	'$' /* this indicates a hexadecimal value */
#define Bin_Notation	'%' /* this indicates a binary value */
#define Comment_char    ';' /* indicates a comment */

#define ARG_NONE       0 
#define ARG_IMMEDIATE  1
#define ARG_SYMBOL     2
#define ARG_SPECIAL    3

#define INST_OPC       0  /* a opcode */ 
#define INST_ORG	   1  /* organization */
#define INST_INCBIN    2  /* includes a binary file */
#define INST_DATA      3  /* data (DN,DB) */

#ifndef __GLOBAL__
#define __GLOBAL__

/* list of symbols */
struct SSymbol
{
	char *name;
	word value;
	byte defined;
};

/* list of files */
struct SFile
{
	char *name;
};

/* list of instructions */
struct SInst
{
	byte *data;
	struct SInst *next;
	byte file;
	word line;
	byte type;
};

struct SOpc
{
	byte opcode;
	byte arg_type;
	word arg;	
};

struct SData
{
	word nibble_size;
	word count;
	byte *data;
};

int ShowError( const char *error, ... );
void AddOPC( byte file, word line, byte opcode, byte arg_type, word arg );
void AddINCBIN( byte file, word line, const char *file_name );
void AddDATA( byte file, word line, word nibble_size, word count, byte *data );
void AddORG( byte file, word line, word address );

#endif

/* Global variables */
extern byte display_logo;
extern byte verbose_mode;
extern byte show_symbols;
extern byte input_file_set;
extern byte output_file_set;
extern word line_count;
extern char *input_file;
extern char *output_file;
extern char current_text_line[MAX_LINE_SIZE];
extern word current_line;
extern byte current_file;
extern word program_address;
extern clock_t t_start, t_end;
extern word error_count;
extern struct SSymbol Symbols[MAX_SYMBOLS];
extern word symbol_count;
extern struct SFile Files[MAX_FILES];
extern byte file_count;
extern struct SInst *first;
extern struct SInst *last;
