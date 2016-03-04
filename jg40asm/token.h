/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/
#include "global.h"

#define TOKEN_NONE               0
#define TOKEN_LABEL              1
#define TOKEN_IDENTIFIER         2
#define TOKEN_KEYWORD            3
#define TOKEN_ARGUMENT           4
#define TOKEN_NUMERIC_VALUE      5
#define TOKEN_DATA_ARGUMENT      6
#define TOKEN_STRING             7
#define TOKEN_SEPARATOR			 8
#define TOKEN_INVALID            9

#ifndef __TOKEN__
#define __TOKEN__

int GetToken( char **string );

#endif

extern char token[MAX_LINE_SIZE];
extern char last_token[MAX_LINE_SIZE];
