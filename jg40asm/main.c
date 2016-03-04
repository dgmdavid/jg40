/*
	JGA40SM - Jaguar Virtual Video-Game Assembler
	version 1.0

	by David G. Maziero
	https://twitter.com/dgmdavid/

	This is a _very_ simple assembler to the JG40 microprocessor.

	Aspects of this assembler:
		label:  -> This is a label
		15      -> Notation for decimal numbers
		$F      -> Notation for hexadecimal numbers
		#1010   -> Notation for binary numbers
		org $FF -> Organize
		include -> Includes a file
		incbin  -> Includes a binary file
		dn, db  -> Declare nibble, declare byte


	Rev 1.0 08/20/2007 -- Finished.
	Rev 0.1 06/30/2007 -- Begin of work.
*/
#include "global.h"
#include "util.h"
#include "token.h"
#include "opcodes.h"


/* PrintLogo - Print the credit message */
void PrintLogo( void )
{
	printf( "Simple Assembler for Jaguar Virtual Video-Game JG40 Microprocessor v%d.%d\n", version_major, version_minor );
	printf( "Developed by David G. Maziero\n\n" );
}


/* PrintHelp - Print the command-line options */
void PrintHelp( void )
{
	printf( "\tHow to use:\n" );
	printf( "\t  jg40asm source.jg4 [-o binary.bin] [-options]\n\n" );
	printf( "\tOptions are:\n" );
	printf( "\t  -o  : Specify the output file\n" );
	printf( "\t  -nl : Display no logo\n" );
	printf( "\t  -s  : List defined symbols\n" );
	printf( "\t  -nv : Not in Verbose mode\n" );
	printf( "\n" );
}


/* ParseArgs - Parse all the command-line options */
int ParseArgs( int argc, char **argv )
{
	int l;
	for( l=1; l<argc; l++ )
	{
		/* options */
		if( argv[l][0]=='-' )
		{
			if( argv[l][1]=='n' )
			{
				/* no logo */
				if( argv[l][2]=='l')
				{
					display_logo = 0;
				} else
				/* no verbose */
				if( argv[l][2]=='v')
				{
					verbose_mode = 0;
				}
			} else 
			if( argv[l][1]=='s' )
			{
				show_symbols = 1;
			} else
			if( argv[l][1]=='o' )
			{
				/* output file */
				++l;
				if( l>=argc ) break;
				if( output_file!=NULL ) free( output_file );
				output_file = (char*)malloc( strlen(argv[l])+1 );
				strcpy( output_file, argv[l] );
				output_file_set = 1;
				continue;
			}
		} else {
			/* input file, I suppose */
			if( input_file!=NULL ) free( input_file );
			input_file = (char*)malloc( strlen(argv[l])+1 );
			strcpy( input_file, argv[l] );
			input_file_set = 1;
		}
	}

	if( display_logo ) PrintLogo();

	/* no input file? */
	if( input_file_set==0 )
	{
		PrintHelp();
		printf( "\tNo input file specified.\n\n" );
		return ERROR;
	} else {
		/* verify if the file exists */
		FILE *f = fopen( input_file, "r" );
		if( f==NULL ) 
		{
			printf( "\tInput file \"%s\" not found.\n\n", input_file );
			return ERROR;
		}
		fclose( f );
	}

	/* no output file? */
	if( output_file_set==0 )
	{
		int l, s = strlen( input_file );
		for( l=0; l<s; ++l )
		{
			if( input_file[l]=='.' ) {
				input_file[l]='\0';
				break;
			}
		}
		output_file = (char*)malloc( l+5 );
		strcpy( output_file, input_file );
		strcat( output_file, ".bin" );
		//printf("out: [%s]\n", output_file);		
		input_file[l]='.';
	}

	return TRUE;
}


/* forward declaration */
int ParseFile( const char *file );

/* ParseDeclare - parse DN and DB values sequence */
int ParseDeclare( int size, char **string )
{
	int type;
	byte first = 0;
	byte data[MAX_LINE_SIZE];
	word datapos = 0;

pds:;
	type = GetToken( string );
	if( type==TOKEN_NONE ) 
	{
		if( datapos==0 )
		{
			return ShowError( "DN/DB expects at least one argument." );
		} else {
			if( size==0 )
			{
				AddDATA( current_file, current_line, 1, datapos, data );
			} else {
				AddDATA( current_file, current_line, 2, datapos, data );
			}
		}
		return 0;
	}

	if( first!=0 )
	{
		if( type!=TOKEN_SEPARATOR ) 
		{
			return ShowError( "Comma-separated values are expected." );
		}
		first = 0;
		goto pds;
	}

	if( type==TOKEN_NUMERIC_VALUE )
	{
		int value = ParseValue( token );
		
		if( value!=ERROR )
		{
			if( size==0 ) if( value>=16 ) value=ERROR;
			if( size==1 ) if( value>=256) value=ERROR;
		}

		if( value==ERROR )
		{
			if( size==0 )
				return ShowError( "DN expects a 4-bit value." );
			else 
				return ShowError( "DB expects a 8-bit value." );
		} else {

			/* add this value to the list */
			data[datapos] = (byte)value;
			datapos++;
			/* increment the program address */
			if( size==0 ) 
				program_address++;
			else
				program_address+=2;
		}
	} else
	
	if( type==TOKEN_STRING )
	{
		int r, s = strlen( token );
		if( size==0 ) return ShowError( "DN expects a 4-bit value." );

		for( r=0; r<s; r++ )
		{
			/* add to the list */
			data[datapos] = (byte)token[r];
			datapos++;
			/* increment the program address */
			program_address+=2;
		}
	
	} else {
		printf( "[%s]", token );
		return ShowError( "Invalid value for DN/DB." );
	}

	first = 1;
	goto pds;

	return 0;
}


/* ParseTokens - translate token to instructions */
int ParseTokens( void )
{
	word berr;
	int type;
	char *string;
	string = current_text_line;

parse_start:;
	berr = error_count;

	type = GetToken( &string );

	/* the line is empty? */
	if( type==TOKEN_NONE ) return TRUE;

	/* is it a label? */
	if( type==TOKEN_LABEL )
	{
		/* add new label */
		AddSymbol( token, program_address, 1 );
		goto parse_start;

	} else

	/* is it an identifier? */
	if( type==TOKEN_IDENTIFIER )
	{
		/* see if it is an operator */
		int _operator = FindOperator( token );

		/* it is an operator */
		if( _operator!=ERROR )
		{
			/* increment program address */
			program_address += Opcodes[_operator].size; 
			if( program_address>MAX_ROM_NSIZE )
			{
				return ShowError( "ROM image exceeded its limit of %d bytes.", MAX_ROM_BSIZE );
			}

			/* if it expects an argument */
			if( Opcodes[_operator].size>1 )
			{
				/* get the argument */
				type = GetToken( &string );
			
				/* no argument? bad */
				if( type==TOKEN_NONE )
				{
					ShowError( "Operand expects an argument." );
					goto parse_end;
				}

				/* check if it's valid */
				if( type==TOKEN_NUMERIC_VALUE ) 
				{
					/* parse value  */
					int value = ParseValue( token );
					if( CheckValue( value, Opcodes[_operator].size )!=ERROR )
					{
						/* add the instruction */
						AddOPC( current_file, current_line, (byte)_operator, ARG_IMMEDIATE, (word)value );
						goto parse_end;
					}
				} else {
					/* it isn't a numeric value, so it's a symbol */
					int find = FindSymbol( token, 0, 1 );
					if( find==ERROR )
					{
						/* it doesn't exists, so add it undefined to the list as look-up */
						int index = AddSymbol( token, 0, 0 );
						if( index!=ERROR )
						{
							AddOPC( current_file, current_line, (byte)_operator, ARG_SYMBOL, (word)index );
						}
						goto parse_end;
					} else {
						/* is symbol defined? get its value */
						int value = CheckValue( Symbols[find].value, Opcodes[_operator].size );
						if( value!=ERROR )
						{
							/* add the instruction */
							AddOPC( current_file, current_line, (byte)_operator, ARG_IMMEDIATE, (word)value );
							goto parse_end;
						}
					}
				}
			} else {
				/* no, it doesn't expect an argument, add the instruction */
				AddOPC( current_file, current_line, (byte)_operator, ARG_NONE, 0 );
			}

		} else {
			/* not an operator? so it should be a definition or an invalid operator */
			/* backup last token */
			strcpy( last_token, token );

			type = GetToken( &string );
			if( type!=TOKEN_NONE )
			{
				if( (strcmp( token, "EQU" )==0) || (strcmp( token, "=" )==0) )
				{
					/* get value */
					type = GetToken( &string );
					if( type==TOKEN_NUMERIC_VALUE ) 
					{
						/* parse value and add nem symbol */
						int value = ParseValue( token );
						if( value!=ERROR )
						{
							AddSymbol( last_token, (word)value, 1 );
							goto parse_end;
						}
					} 
					ShowError( "Invalid value." );
					goto parse_end;
				} 
			}
			ShowError( "Invalid identifier." );
		}
	} else

	/* is it a keyword? */
	if( type==TOKEN_KEYWORD )
	{
		/* INCLUDE */
		if( token[0]==0 )
		{
			/* get the parameter */
			type = GetToken( &string );
			if( type!=TOKEN_STRING ) 
			{
				if( berr==error_count )	ShowError( "Include expects a string." );
				return TRUE;
			}
			/* parse the file */
			if( strcmp( Files[current_file].name, token )!=0 )
			{
				byte back_file = current_file;
				word back_line = current_line;

				if( ParseFile( token )==ERROR )
				{
					return ERROR;
				}
				
				current_file = back_file;
				current_line = back_line;
			} else {
				ShowError( "Including the same file as actual is illegal." );
			}
		} else

		/* INCBIN */
		if( token[0]==1 )
		{
			FILE *f = NULL;
			//int file_name_index = 0;

			/* get the parameter */
			type = GetToken( &string );
			if( type!=TOKEN_STRING ) 
			{
				if( berr==error_count )	ShowError( "Incbin expects a string." );
				return TRUE;
			}

			f = fopen( token, "rb" );
			if( f==NULL )
			{
				ShowError( "File not found." );
			} else {
				long file_size;
				fseek( f, 0, SEEK_END );
				file_size = ftell( f );

				if( (program_address+(file_size*2))>MAX_ROM_NSIZE )
				{
					return ShowError( "ROM image will exceed its limit of %d bytes.", MAX_ROM_BSIZE );
				}
				AddINCBIN( current_file, current_line, token );
				fclose( f );
				/* increment program address */
				program_address += (word)(file_size*2);
			}
		} else

		/* ORG */
		if( token[0]==2 )
		{
			int address = -1;
		
			type = GetToken( &string );
			if( type==TOKEN_NUMERIC_VALUE ) address = ParseValue( token );
			
			if( address==-1 )
			{
				ShowError( "ORG expects a 12-bit value." );
			} else {
				if( address>MAX_ROM_NSIZE )
				{
					return ShowError( "Organization beyond maximum ROM size of %h nibbles.", MAX_ROM_NSIZE );
				} else
				if( address<program_address )
				{
					return ShowError( "Cannot organize backwards." );
				}
				AddORG( current_file, current_line, (word)address );
				/* set program address */
				program_address = (word)address;
			}

		} else

		/* DN */
		if( token[0]==3 )
		{
			ParseDeclare( 0, &string );
		} 

		/* DB */
		if( token[0]==4 )
		{
			ParseDeclare( 1, &string );
		} 

	} else {
		
		ShowError( "Unknown/Invalid identifier." );
		return TRUE;
	}

parse_end:;

	/* more tokens remaining on the line? */
	if( berr==error_count )
	{
		type = GetToken( &string );
		if( type!=TOKEN_NONE )
		{
			ShowError( "Illegal parameter." );
		}
	}

	return TRUE;
}


/* ParseFile - translate file to opcodes, search for symbols */
int ParseFile( const char *file )
{
	byte index;
	FILE *f;

	f = fopen( file, "rt" );
	if( !f ) 
	{
		return ShowError( "File not found <%s>.", file );
	}

	index = AddFile( file );
	if( index!=ERROR )
	{
		current_file = index;
		current_line = 0;
		while( !feof( f ) )
		{
			if( fgets( current_text_line, MAX_LINE_SIZE, f ) )
			{
				current_line++;
				line_count++;
			
				if( strlen( current_text_line )==MAX_LINE_SIZE-1 )
				{
					if( (current_text_line[MAX_LINE_SIZE-1]!='\n') || (current_text_line[MAX_LINE_SIZE-1]!='\r') )
					{
						ShowError( "Line too long." );
						continue;
					}
				}
				if( ParseTokens()==ERROR ) return FALSE;
			}
		}
	}

	fclose(f);

	return TRUE;
}


/* writeROM */
void writeROM4bit( byte *ROM, word address, byte d_nibble )
{
	word dst_address = (address>>1);
	word dst_nibble = address-(dst_address<<1);

	if( dst_nibble==0 )
	{
		ROM[dst_address] |= (d_nibble<<4);
	} else {
		ROM[dst_address] |= (d_nibble & 15);
	}
}

void writeROM8bit( byte *ROM, word address, byte d_byte )
{
	writeROM4bit( ROM, (word)address,     (byte)(d_byte>>4)   );
	writeROM4bit( ROM, (word)(address+1), (byte)(d_byte & 15) );
}

void writeROM12bit( byte *ROM, word address, word d_12bit )
{
	writeROM4bit( ROM, (word)address,     (byte)(d_12bit>>8)        );
	writeROM4bit( ROM, (word)(address+1), (byte)((d_12bit>>4) & 15) );
	writeROM4bit( ROM, (word)(address+2), (byte)(d_12bit & 15)      );
}


/* Assemble - assemble the and save the ROM image */
int Assemble( void )
{
	struct SInst *I = first;
	word bytes_saved = 0;
	byte ROM[MAX_ROM_BSIZE];

	memset( &ROM, 0, MAX_ROM_BSIZE );
	
	program_address = START_ADDRESS;
	
	while( I )
	{
		int value = -1;
	
		/* this instruction is an opcode */
		if( I->type==INST_OPC )
		{
			struct SOpc *opc = (struct SOpc*)I->data;
			
			if( opc->arg_type==ARG_NONE )
			{
				value = 0;
			} else

			if( opc->arg_type==ARG_IMMEDIATE )
			{
				value = opc->arg;
			} else

			if( opc->arg_type==ARG_SYMBOL )
			{
				/* check if the symbol is defined */
				if( Symbols[opc->arg].defined==0 )
				{
					current_file = I->file;
					current_line = I->line;
					ShowError( "Undefined symbol." );
				} else {
					value = Symbols[opc->arg].value;
					/* check if the value is valid for the opcode */
					current_file = I->file;
					current_line = I->line;
					if( CheckValue( value, Opcodes[opc->opcode].size )==ERROR ) value = -1;
				}
			}
	
			if( value!=-1 )
			{
				/* write the opcode to the ROM image */
				writeROM4bit( ROM, program_address, opc->opcode );
			
				/* write its parameters */
				if( Opcodes[opc->opcode].size==2 )
				{
					writeROM4bit( ROM, (word)(program_address+1), (byte)value );
				} else 			
				if( Opcodes[opc->opcode].size==3 )
				{
					writeROM8bit( ROM, (word)(program_address+1), (byte)value );
				} else 			
				if( Opcodes[opc->opcode].size==4 )
				{
					writeROM12bit( ROM, (word)(program_address+1), (word)value );
				}
			}

			/* increment program address */
			program_address += Opcodes[opc->opcode].size;
		} else

		/* this instruction is incbin */
		if( I->type==INST_INCBIN )
		{
			FILE *f;
			f = fopen( (const char*)I->data, "rb" );
			if( f )
			{
				byte db;
				while( !feof(f) )
				{
					if( fread( &db, 1, 1, f )>0 )
					{
						writeROM8bit( ROM, (word)program_address, db );
						program_address+=2;
					}
				}
				fclose( f );
			}
		} else

		/* this instruction is data */
		if( I->type==INST_DATA )
		{
			int r;
			struct SData *S;
			S = (struct SData*)I->data;
			
			if( S->nibble_size==1 )
			{
				for( r=0; r<S->count; r++ )
				{
					writeROM4bit( ROM, (word)program_address, S->data[r] );
					program_address++;
				}
			} else {
				for( r=0; r<S->count; r++ )
				{
					writeROM8bit( ROM, (word)program_address, S->data[r] );
					program_address+=2;
				}
			}
		} else

		/* this instruction is ORG */
		if( I->type==INST_ORG )
		{
			word *address = (word*)I->data;
			program_address = *address;
		}

		/* get the next instruction */
		I = I->next;
	}

	//calculate the size, in bytes, of the ROM image
	bytes_saved = ((program_address-START_ADDRESS)>>1)+((program_address-START_ADDRESS)&1);

	if( bytes_saved>0 )
	{
		/* save the ROM file */
		FILE *f;
		f = fopen( output_file, "wb" );
		fwrite( &ROM, 1, bytes_saved, f );
		fclose( f );

	}

	return bytes_saved;
}


/* Main function */
int main( int argc, char *argv[] )
{
	int return_value = 0;
	int saved_bytes = 0;

	/* parse the command-line options */
	if( ParseArgs( argc, argv )==ERROR ) return ERROR;

	/* begin the measurement of the processing time */
	t_start = clock();

	/* first pass, define symbols and labels */
	program_address = START_ADDRESS;
	ParseFile( input_file );

	/* if no erros, assemble the file */
	if( error_count==0 )
	{
		saved_bytes = Assemble();
	}

	if( saved_bytes==ERROR ) 
	{
		saved_bytes = 0;
		return_value = ERROR;
	}

	/* finish the measurement of the processing time */
	t_end = clock();

	/* print the stats */
	if( verbose_mode )
	{
		if( error_count>0 )
		{
			printf( "\nFailed, %d errors.\n", error_count );
		} else {
			printf( "\nSuccess.\n" );
			printf( " %d symbol(s) defined.\n", symbol_count );
		}
		printf( " %d lines of code.\n", line_count );
		printf( " %d bytes saved.\n", saved_bytes );
		printf( " %d.%03ds total processing time.\n\n", (int)((t_end-t_start)/CLK_TCK), (int)((t_end-t_start)%CLK_TCK) );

	}

	/* free memory */
	if( input_file ) free( input_file );
	if( output_file ) free( output_file );

	/* free symbol and file table */
	if( symbol_count>0 )
	{
		word l;
		if( show_symbols ) printf( "Symbol(s):\n");
		for( l=0; l<symbol_count; l++ )
		{
			if( show_symbols ) printf( "\t[%s=$%03X]\n", Symbols[l].name, Symbols[l].value );
			free( Symbols[l].name );
		}
	}
	if( file_count>0 )
	{
		byte l;
		if( show_symbols ) printf( "File(s):\n");
		for( l=0; l<file_count; l++ )
		{
			if( show_symbols ) printf( "\t[%s]\n", Files[l].name );
			free( Files[l].name );
		}
	}

	/* free the instruction sequence */
	{
		struct SInst *I = first;
		while( I )
		{
			struct SInst *next = I->next;
			if( I->type==INST_DATA )
			{
				struct SData *D = (struct SData*)I->data;
				free( D->data );
			}
			free( I->data );
			free( I );
			I = next;
		}
	}

	return return_value;
}
