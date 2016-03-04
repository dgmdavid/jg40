/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/
#include "global.h"

#ifndef __UTIL__
#define __UTIL__

int FindOperator( const char *mnemonic );
int FindKeyword( const char *kword );
int AddSymbol( const char *symbol, word value, byte define );
int FindSymbol( const char *symbol, byte show_error, byte find_defined );
int AddFile( const char *name );

int IsNotSpace( const char c );
int IsAlpha( const char c );
int IsDigit( const char c );
int IsHex( const char c );
int IsBin( const char c );
int IsComment( const char c );

int ParseValue( const char *val );
int CheckValue( int value, byte nibble_size );

int DecToWord( const char *dec );
int HexToWord( const char *hex );
int BinToWord( const char *binary );

int ValidateIncludeName( char *param );

#endif
