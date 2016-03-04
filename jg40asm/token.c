/*
	JG40ASM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/
*/
#include <ctype.h>
#include "token.h"
#include "util.h"

char token[MAX_LINE_SIZE];
char last_token[MAX_LINE_SIZE];


/* GetToken */
int GetToken( char **string )
{
	char *token_start = NULL;
	word token_size=0;

	while( **string!='\0' )
	{
		/* is it a separator? (comma) */
		if( **string==',' )
		{
			*string = *string+1;
			token[0] = ',';
			token[1] = '\0';
			return TOKEN_SEPARATOR;
		}

		if( IsNotSpace( **string ) )
		{
			/* is it a comment? */
			if( IsComment( **string ) ) return TOKEN_NONE;

			**string = (char)toupper( **string );
			token_start = *string;

			/* see if it's a string */
			if( **string=='"' ) 
			{
				*string = *string+1;
				/* find the end of the string */
				while( **string!='\0' )
				{
					*string = *string+1;
					token_size++;
					if( **string=='"' )
					{
						/* copy token without the quotes */
						if( token_size>1 )
						{
							memcpy( token, token_start+1, token_size );
							token[token_size] = '\0';
						} else {
							token[0]='\0';
						}
						*string = *string+1;
						return TOKEN_STRING;
					}
				}
				ShowError( "Unclosed string." );
				return TOKEN_INVALID;
			}

			/* find the end of the word */
			while( IsNotSpace( **string ) )
			{
				**string = (char)toupper( **string );
				*string = *string+1;
				token_size++;
				if( **string=='\0' ) break;
			}

			/* copy token */
			memcpy( token, token_start, token_size );
			token[token_size] = '\0';

			/* is it a label? */
			if( token[token_size-1]==':' ) 
			{
				if( !IsAlpha(token[0]) ) return TOKEN_INVALID;
				token[token_size-1] = '\0';
				return TOKEN_LABEL;
			}

			/* is it a numeric value? */
			if( IsDigit( token[0] ) ||
				IsHex( token[0] )   ||
				IsBin( token[0] )      ) return TOKEN_NUMERIC_VALUE;

			/* is it a keyword? */
			{
				int kw = FindKeyword( token );
				if( kw!=ERROR )
				{
					token[0] = (char)kw;
					return TOKEN_KEYWORD;
				}
			}

			return TOKEN_IDENTIFIER;
		}
		
		*string = *string+1;
	}

	return TOKEN_NONE;
}
