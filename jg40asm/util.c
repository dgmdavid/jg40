/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/

#include "util.h"
#include "opcodes.h"


/* FindOperator */
int FindOperator( const char *mnemonic )
{
	byte l;
	for( l=0; l<NUM_OPCODES; ++l )
	{
		if( strcmp( mnemonic, Opcodes[l].mnemonic )==0 )
		{
			return l;
		}
	}
	return ERROR;
}


/* FindKeyword */
int FindKeyword( const char *kword )
{
	byte l;
	for( l=0; l<NUM_KEYWORDS; ++l )
	{
		if( strcmp( kword, Keywords[l] )==0 )
		{
			return l;
		}
	}
	return ERROR;
}


/* AddSymbol */
int AddSymbol( const char *symbol, word value, byte define )
{
	int index;

	if( symbol_count==MAX_SYMBOLS )
	{
		return ShowError( "You cannot define more than %d symbols.", MAX_SYMBOLS );
	}
	
	index = FindSymbol( symbol, 0, 0 );

	/* the symbols doesn't exist */
	if( index==ERROR )
	{
		/* create and define it */
		Symbols[symbol_count].name = (char*)malloc( strlen(symbol)+1 );
		strcpy( Symbols[ symbol_count ].name, symbol );
	
		Symbols[symbol_count].value = value;
		Symbols[symbol_count].defined = define;
	} else {
		/* it already exists and it's defined, error */
		if( Symbols[index].defined==1 )
			return ShowError( "Duplicated symbol." );

		/* it already exists, but it isn't defined, define it if needed */
		Symbols[index].defined = define;
		Symbols[index].value = value;
		return index;
	}
	
	return symbol_count++;
}


/* FindSymbol */
int FindSymbol( const char *symbol, byte show_error, byte find_defined )
{
	int l;
	if( symbol_count==0 ) goto fsymbol_end;
	for( l=0; l<symbol_count; ++l )
	{
		if( strcmp( symbol, Symbols[l].name )==0 )
		{
			if( find_defined==1 ) 
			{
				if( Symbols[l].defined==1 )	return l;
			} else {
				return l;
			}
		}
	}

fsymbol_end:
	if( show_error==1 )
		return ShowError( "Undefined symbol." );
	else
		return ERROR;
}


/* AddFile */
int AddFile( const char *name )
{
	if( file_count==MAX_FILES )
	{
		return ShowError( "You cannot include more than %d files.", MAX_FILES );
	}

	Files[file_count].name = (char*)malloc( strlen(name)+1 );
	strcpy( Files[file_count].name, name );

	return file_count++;
}


/* IsNotSpace */
int IsNotSpace( const char c )
{
	if( c!=' ' && c!='\t' && c!='\n' && c!='\r' && c!=',' ) return TRUE;
	return FALSE;
}


/* IsAlpha */
int IsAlpha( const char c )
{
	if( c>='A' && c<='Z' ) return TRUE;
	return FALSE;
}



/* IsDigit */
int IsDigit( const char c )
{
	if( c>='0' && c<='9' ) return TRUE;
	return FALSE;
}


/* IsHex */
int IsHex( const char c )
{
	if( c==Hex_Notation ) return TRUE;
	return FALSE;
}


/* IsBin */
int IsBin( const char c )
{
	if( c==Bin_Notation ) return TRUE;
	return FALSE;
}

/* IsComment */
int IsComment( const char c )
{
	if( c==Comment_char ) return TRUE;
	return FALSE;
}


/* DecToWord */
int DecToWord( const char *dec )
{
	int l;
	int value;
	for( l=0; dec[l]!='\0'; ++l )
	{
		if( !IsDigit( dec[l] ) ) 
		{
			return ShowError( "Invalid decimal value." );
		}
	}

	value = atoi( dec );
	if( value>=MAX_VALUE ) return ERROR;

	return value;
}


/* HexToWord */
int HexToWord( const char *hex )
{
	int l=strlen(hex)-1;
	int value=0, t=1;
	for( ; l>0; --l )
	{
		if( hex[l]=='0' ) 
		{
			/* do nothing */
		} else
		if( hex[l]=='1' ) value+=1*t; else
		if( hex[l]=='2' ) value+=2*t; else
		if( hex[l]=='3' ) value+=3*t; else
		if( hex[l]=='4' ) value+=4*t; else
		if( hex[l]=='5' ) value+=5*t; else
		if( hex[l]=='6' ) value+=6*t; else
		if( hex[l]=='7' ) value+=7*t; else
		if( hex[l]=='8' ) value+=8*t; else
		if( hex[l]=='9' ) value+=9*t; else
		if( hex[l]=='A' ) value+=10*t; else
		if( hex[l]=='B' ) value+=11*t; else
		if( hex[l]=='C' ) value+=12*t; else
		if( hex[l]=='D' ) value+=13*t; else
		if( hex[l]=='E' ) value+=14*t; else
		if( hex[l]=='F' ) value+=15*t; 
		else return ShowError( "Invalid hexadecimal value." );
		t *= 16;

		if( value>=MAX_VALUE ) return ERROR;
	}
	return value;
}


/* BinToWord */
int BinToWord( const char *binary )
{
	int l=strlen(binary)-1;
	int value=0, t=1;
	for( ; l>0; --l )
	{
		if( binary[l]!='0' && binary[l]!='1' ) 
			return ShowError( "Invalid binary value." );
		if( binary[l]=='1' ) value+=t;
		t *= 2;

		if( value>=MAX_VALUE ) return ERROR;
	}
	return value;
}


/* ValidateIncludeName */
int ValidateIncludeName( char *param )
{
	/* does it start with quotes? */
	if( param[0]=='"' )
	{
		/* find for quote close */
		int l,s;
		s = strlen(param);
		for( l=1; l<s; ++l ) if( param[l]=='"' ) break;
		if( l<s-1 ) return FALSE;
		/* ok, remove it */
		for( l=0; l<s-1; ++l ) param[l] = param[l+1];
		param[s-2]='\0';
		return TRUE;
	}
	return FALSE;
}


/* ParseValue */
int ParseValue( const char *val )
{
	/* decimal */
	if( IsDigit( val[0] ) )
	{
		return DecToWord( val );
	} else

	/* hexadecimal */
	if( IsHex( val[0] ) )
	{
		return HexToWord( val );
	} else

	/* binary */
	if( IsBin( val[0] ) )
	{
		return BinToWord( val );
	} else return ERROR;
}


/* CheckValue */
int CheckValue( int value, byte nibble_size )
{
	if( value==ERROR ) value = 0xFFFF;

	if( nibble_size==2 )
	{
		if( value>=16 ) 
		{
			return ShowError( "Operator expects a 4-bit value as argument." );
		}
	} else

	if( nibble_size==3 )
	{
		if( value>=256 ) 
		{
			return ShowError( "Operator expects an 8-bit value as argument." );
		}
	} else

	if( nibble_size==4 )
	{
		if( value>=4096 ) 
		{
			return ShowError( "Operator expects an 12-bit value as argument." );
		}
	}

	return value;
}
