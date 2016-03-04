/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/
#include "global.h"
#include "stdarg.h"

/* Global variables */
byte display_logo    = 1;
byte verbose_mode    = 1;
byte show_symbols    = 0;
byte input_file_set  = 0;
byte output_file_set = 0;
word line_count     = 0;
word error_count     = 0;
word symbol_count    = 0;
byte file_count      = 0;
word program_address = 0;
char *input_file     = NULL;
char *output_file    = NULL;
char current_text_line[MAX_LINE_SIZE];
word current_line    = 0;
byte current_file    = 0;
clock_t t_start, t_end;
struct SSymbol Symbols[MAX_SYMBOLS];
struct SFile Files[MAX_FILES];
struct SInst *first = NULL;
struct SInst *last = NULL;


/* ShowError */
int ShowError( const char *error, ... )
{
	char tmp[1024];

	va_list	ap;
	va_start( ap, error );
	vsprintf( tmp, error, ap );
	va_end( ap );

	printf("[%s] Error on line %d: %s\n", Files[current_file], current_line, tmp );

	error_count++;

	return ERROR;
}


/* AddOPC */
void AddOPC( byte file, word line, byte opcode, byte arg_type, word arg )
{
	struct SInst *I;
	struct SOpc *opc;

	opc = malloc( sizeof(struct SOpc) );
	opc->opcode = opcode;
	opc->arg_type = arg_type;
	opc->arg = arg;

	I = malloc( sizeof(struct SInst) );
	I->file = file;
	I->line = line;
	I->type = INST_OPC;
	I->data = (byte*)opc;
	I->next = NULL;

	if( first==NULL )
	{
		first = I;
		last = first;
	} else {
		last->next = I;
		last = I;
	}
}


/* AddINCBIN */
void AddINCBIN( byte file, word line, const char *file_name )
{
	struct SInst *I;

	I = malloc( sizeof(struct SInst) );
	I->file = file;
	I->line = line;
	I->type = INST_INCBIN;
	
	I->data = malloc( strlen(file_name)+1 );
	strcpy( (char*)I->data, file_name );
	
	I->next = NULL;

	if( first==NULL )
	{
		first = I;
		last = first;
	} else {
		last->next = I;
		last = I;
	}
}


/* AddDATA */
void AddDATA( byte file, word line, word nibble_size, word count, byte *data )
{
	struct SInst *I;
	struct SData *D;

	I = malloc( sizeof(struct SInst) );
	I->file = file;
	I->line = line;
	I->type = INST_DATA;
	I->next = NULL;

	D = malloc( sizeof(struct SData) );
	D->nibble_size = nibble_size;
	D->count = count;

	D->data = malloc( count );
	memcpy( D->data, data, count );

	I->data = (byte*)D;

	if( first==NULL )
	{
		first = I;
		last = first;
	} else {
		last->next = I;
		last = I;
	}
}


/* AddORG */
void AddORG( byte file, word line, word address )
{
	struct SInst *I;
	word *a = malloc( sizeof(word) );
	*a = address;

	I = malloc( sizeof(struct SInst) );
	I->file = file;
	I->line = line;
	I->type = INST_ORG;
	I->data = (byte*)a;
	I->next = NULL;

	if( first==NULL )
	{
		first = I;
		last = first;
	} else {
		last->next = I;
		last = I;
	}
}
