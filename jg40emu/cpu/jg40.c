/*
	JG40 - Jaguar Virtual Video-Game
	
	JG40 CPU emulator

	by David G. Maziero
	http://www.dgmsoft.rg3.net/jaguar.html

	Rev 0.1 17/10/2007 -- Begin of work.
*/

#include "memory.h"
#include "jg40.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "conio.h"


byte OPCODE_CYCLES[16] = {	
		/* LDA */  2,		/* STA */  2,
		/* ADD */  4,		/* OR  */  3,
		/* AND */  3,		/* NOT */  2,
		/* SHL */  2,		/* SHR */  2,
		/* PUA */  4,		/* POA */  4,
		/* CMP */  4,		/* JZ  */  8,
		/* JC  */  8,		/* SED */  5,
		/* SES */  7,		/* TRR */  6
	};

char OPCODE_NAMES[16][4] = {
		"LDA",		"STA",		"ADD",		"OR ",
		"AND",		"NOT",		"SHL",		"SHR",
		"PUA",		"POA",		"CMP",		"JZ ",
		"JC ",		"SED",		"SES",		"TRR",
	};

int cycle_count;

byte (*MMIOCB_write)(byte) = 0;
byte (*MMIOCB_read)() = 0;


/* jg40_initialize 
   Initializes the JG40 CPU. 
*/
void jg40_initialize()
{
	/* clear up the memory */
	memset( &jg40.RAM, 0, RAM_SIZE>>1 );
	memset( &jg40.SRAM, 0, SRAM_SIZE>>1 );
	memset( &jg40.VRAM, 0, VRAM_SIZE ); /* "unpacked" */
	memset( &jg40.SNDRAM, 0, SNDRAM_SIZE ); /* "unpacked" */
	memset( &jg40.ROM, 0, ROM_SIZE>>1 );

	/* set pointers to zero */
	jg40_reset();
}


/* jg40_loadROM 
   Loads a ROM image.
*/
int jg40_loadROM( byte *ROM, int size )
{
	if( size<=ROM_SIZE )
	{
		jg40.loaded_ROM_size = size*2;
		memcpy( &jg40.ROM, ROM, size );
		return TRUE;
	} else {
		return FALSE;
	}
}


/* jg40_loadstate
   Loads a full CPU state, except the ROM image.
*/
void jg40_loadstate( struct JG40_CPU *state )
{
	memcpy( jg40.RAM, state->RAM, RAM_SIZE>>1 );
	memcpy( jg40.SRAM, state->SRAM, SRAM_SIZE>>1 );
	memcpy( jg40.VRAM, state->VRAM, VRAM_SIZE ); /* "unpacked" */
	memcpy( jg40.SNDRAM, state->SNDRAM, SNDRAM_SIZE ); /* "unpacked" */

	jg40.accumulator = state->accumulator;
	jg40.RAM_pointer = state->RAM_pointer;
	jg40.SRAM_pointer = state->SRAM_pointer;
	jg40.VRAM_pointer = state->VRAM_pointer;
	jg40.ROM_pointer = state->ROM_pointer;
	jg40.program_pointer = state->program_pointer;
	jg40.loaded_ROM_size = state->loaded_ROM_size;
	jg40.zero_flag = state->zero_flag;
	jg40.carry_flag = state->carry_flag;
}


/* j40_reset
   Soft reset the JG40 CPU.
*/
void jg40_reset()
{
	jg40.accumulator = 0;
	jg40.RAM_pointer = 0;
	jg40.SRAM_pointer = 0;
	jg40.VRAM_pointer = 0;
	jg40.ROM_pointer = 0;
	jg40.program_pointer = 0;
	jg40.zero_flag = 0;
	jg40.carry_flag = 0;

	/* clear all memory, but ROM */
	memset( &jg40.RAM, 0, RAM_SIZE>>1 );
	memset( &jg40.SRAM, 0, SRAM_SIZE>>1 );
	memset( &jg40.VRAM, 0, VRAM_SIZE ); /* "unpacked" */
	memset( &jg40.SNDRAM, 0, SNDRAM_SIZE ); /* "unpacked" */

	jg40.finished = FALSE;
}


/* ZERO_FLAG
   Macro to set the zero flag.
*/
#define ZERO_FLAG(a) jg40.zero_flag = a;


/* CARRY_FLAG
   Macro to set the carry flag.
*/
#define CARRY_FLAG(a) jg40.carry_flag = a;


/* jg40_execute
   Executes "cycles" cycles of program.
*/
int jg40_execute( int cycles )
{
	byte opcode;
	word pointer_back;
	cycle_count = 0;

	if( jg40.finished==TRUE ) return TRUE;

	while( cycle_count<cycles )
	{
		/* loads the next opcode */
		opcode = jg40_read4bit( jg40.ROM, jg40.program_pointer );
		
		pointer_back = jg40.program_pointer;
		jg40.program_pointer += 1;

		/* increment the cycles count */
		jg40_takecycles( OPCODE_CYCLES[opcode] );
		
		/* execute the opcode */
		
		/* TODO: optimize this
			     maybe function pointers?
				 maybe memory reading functions to macro?
		*/

		/* LDA - Load A with memory at D. - flags: -Z */
		if( opcode==LDA )
		{
			jg40.accumulator = jg40_read4bit( jg40.RAM, jg40.RAM_pointer );
			ZERO_FLAG( (jg40.accumulator==0) );
		} else
		

		/* STA - Store A in memory at D. - flags: -- */
		if( opcode==STA )
		{
			if( jg40_write4bit( jg40.RAM, jg40.RAM_pointer, jg40.accumulator )>0 )
			{
				/* break the execution */
				break;
			}
		} else


		/* ADD - Add memory value in D to A. - flags: CZ */
		if( opcode==ADD )
		{
			byte value = jg40_read4bit( jg40.RAM, jg40.RAM_pointer );
			
			jg40.accumulator += value;

			if( jg40.accumulator>15 )
			{
				jg40.accumulator -= 16;
				CARRY_FLAG( 1 );
			} else {
				CARRY_FLAG( 0 );
			}
			
			ZERO_FLAG( (jg40.accumulator==0) );
		} else


		/* OR i4 - Logical OR with A. - flags: -Z */
		if( opcode==OR )
		{
			byte i4 = jg40_read4bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer++;

			jg40.accumulator |= i4;
			ZERO_FLAG( (jg40.accumulator==0) );
		} else

	
		/* AND i4 - Logical AND with A. - flags: -Z */
		if( opcode==AND )
		{
			byte i4 = jg40_read4bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer++;

			jg40.accumulator &= i4;
			ZERO_FLAG( (jg40.accumulator==0) );
		} else


		/* NOT - Invert all bits of A. - flags: -Z */
		if( opcode==NOT )
		{
			jg40.accumulator = (~jg40.accumulator)&15;
			ZERO_FLAG( (jg40.accumulator==0) );
		} else 


		/* SHL - Binary left shift in A. - flags: CZ */
		if( opcode==SHL )
		{
			/* carry if the last bit is set */
			CARRY_FLAG( (jg40.accumulator&8) );			
			
			jg40.accumulator = (jg40.accumulator<<1)&15;
			ZERO_FLAG( (jg40.accumulator==0) );
		} else


		/* SHR - Binary right shift in A. - flags: CZ */
		if( opcode==SHR )
		{
			/* carry if the first bit is set */
			CARRY_FLAG( (jg40.accumulator&1) );

			jg40.accumulator = (jg40.accumulator>>1);
			ZERO_FLAG( (jg40.accumulator==0) );
		} else


		/* PUA - Push A to stack. - flags: -- */
		if( opcode==PUA )
		{
			jg40_write4bit( jg40.SRAM, jg40.SRAM_pointer, jg40.accumulator );
			jg40.SRAM_pointer++;
			if( jg40.SRAM_pointer>=SRAM_SIZE ) jg40.SRAM_pointer=0;
		} else


		/* POA - Pop A from stack. - flags: -Z */
		if( opcode==POA )
		{
			if( jg40.SRAM_pointer==0 )
			{
				jg40.SRAM_pointer = SRAM_SIZE-1;
			} else {
				jg40.SRAM_pointer--;
			}
			jg40.accumulator = jg40_read4bit( jg40.SRAM, jg40.SRAM_pointer );
			ZERO_FLAG( (jg40.accumulator==0) );
		} else


		/* CMP - Compare A with memory in D. - flags: CZ */
		if( opcode==CMP )
		{
			/* If A is greater, then Carry flag will be set and Zero flag unset.
			   If A is lesser, then Carry flag will be unset and Zero flag set.
			   If A is equal, then Carry and Zero flag will be unset.
			*/

			byte value = jg40_read4bit( jg40.RAM, jg40.RAM_pointer );

			if( jg40.accumulator>value )
			{
				CARRY_FLAG( 1 );
				ZERO_FLAG( 0 );
			} else
			if( jg40.accumulator<value )
			{
				CARRY_FLAG( 0 );
				ZERO_FLAG( 1 );
			} else {
				CARRY_FLAG( 0 );
				ZERO_FLAG( 0 );
			}
		} else


		/* JZ a12 - Conditional jump if Zero flag is set. - flags: -- */
		if( opcode==JZ )
		{
			word value = jg40_read12bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer += 3;

			if( jg40.zero_flag )
			{
				jg40.program_pointer = value;
			}
		} else


		/* JC a12 - Conditional jump if Carry flag is set. - flags: -- */
		if( opcode==JC )
		{
			word value = jg40_read12bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer += 3;

			if( jg40.carry_flag )
			{
				jg40.program_pointer = value;
			}
		} else


		/* SED i4 -  Set D. - flags: -- */
		if( opcode==SED )
		{
			byte value = jg40_read4bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer += 1;

			jg40.RAM_pointer = value;
		} else


		/* SES i12 - Set S. - flags: -- */
		if( opcode==SES )
		{
			word value = jg40_read12bit( jg40.ROM, jg40.program_pointer );
			jg40.program_pointer += 3;

			jg40.ROM_pointer = value;

		} else
 

		/* TRR - Transfer value in S to RAM in D. Inc S. - flags: -Z */
		if( opcode==TRR )
		{
			byte value = jg40_read4bit( jg40.ROM, jg40.ROM_pointer );
			jg40.ROM_pointer += 1;
			if( jg40.ROM_pointer>ROM_SIZE ) jg40.ROM_pointer = 0;

			jg40_write4bit( jg40.RAM, jg40.RAM_pointer, value );

			ZERO_FLAG( (value==0) );
		} else {

			/* unknown opcode */
			/* this would never happen */
			return FALSE;
		}

		
		/* program finished? 
		   a jg40 program end is detected when you have a jump to the
		   same position of the jump opcode, or when the
		   program_pointer passes the boundary of rom size 
		   this is a workaround
		*/
		if( (pointer_back==jg40.program_pointer) ||
		    (jg40.program_pointer>=ROM_SIZE) )
		{
			/* program finished */
			jg40.program_pointer = ROM_SIZE;
			jg40.finished = TRUE;
			return FALSE;
		}

	}

	return TRUE;
}


/* jg40_setmemorymappedIOcallback
   Sets a callback function to the memory mapped IO write and read
*/
void jg40_setmemorymappedIOcallbacks( byte (*writecallback)(byte), byte (*readcallback)() )
{
	MMIOCB_write = writecallback;
	MMIOCB_read = readcallback;
}


/* jg40_read4bit
   Reads a 4 bit value from memory.
*/
byte jg40_read4bit( byte *MEM, word address )
{
	word dst_address = (address>>1);
	word dst_nibble = address-(dst_address<<1);

	/* check for reading on the memory mapped IO port */
	if( (MEM==jg40.RAM) && (address==MEMORY_MAPPED_IO_ADDRESS) )
	{
		if( MMIOCB_read ) 
		{
			return MMIOCB_read();
		}
	} 

	if( dst_nibble==0 )
	{
		return MEM[dst_address]>>4;
	} else {
		return MEM[dst_address] & 15;
	}
}


/* jg40_read8bit
   Reads a 8 bit value from memory.
*/
byte jg40_read8bit( byte *MEM, word address )
{
	/* TODO: optimize this function */
	return (jg40_read4bit(MEM,address)<<4)|
		   (jg40_read4bit(MEM,(word)(address+1)));
}


/* jg40_read12bit
   Reads a 12 bit value from memory.
*/
word jg40_read12bit( byte *MEM, word address )
{
	/* TODO: optimize this function */
	return (jg40_read4bit(MEM,address)<<8)|
           (jg40_read4bit(MEM,(word)(address+1))<<4)|		
		   (jg40_read4bit(MEM,(word)(address+2)));
}


/* jg40_write4bit
   Writes a 4 bit value to memory.
*/
byte jg40_write4bit( byte *MEM, word address, byte value )
{
	word dst_address = (address>>1);
	word dst_nibble = address-(dst_address<<1);
	
	if( dst_nibble==0 )
	{
		MEM[dst_address] &= 0x0F;
		MEM[dst_address] |= (value<<4);
	} else {
		MEM[dst_address] &= 0xF0;
		MEM[dst_address] |= (value & 15);
	}

	/* check for writing on the memory mapped IO port */
	if( MEM==jg40.RAM )
	{
		if( address==MEMORY_MAPPED_IO_ADDRESS && MMIOCB_write )
		{
			return MMIOCB_write( value );
		}
	}

	return 0;
}


/* jg40_write8bit
   Writes a 8 bit value to memory.
*/
void jg40_write8bit( byte *MEM, word address, byte value )
{
	/* TODO: optimize this function */
	jg40_write4bit( MEM, (word)address,     (byte)(value>>4)   );
	jg40_write4bit( MEM, (word)(address+1), (byte)(value & 15) );
}


/* jg40_write12bit
   Writes a 12 bit value to memory.
*/
void jg40_write12bit( byte *MEM, word address, word value )
{
	/* TODO: optimize this function */
	jg40_write4bit( MEM, (word)address,     (byte)(value>>8)        );
	jg40_write4bit( MEM, (word)(address+1), (byte)((value>>4) & 15) );
	jg40_write4bit( MEM, (word)(address+2), (byte)(value & 15)      );
}



/* jg40_takecycles
   Consume "cycles" number of CPU clock cycles
*/
void jg40_takecycles( int cycles )
{
	cycle_count += cycles;
}


/* jg40_getinstruction
   Translates an opcode and its argument to a string instruction.
   "inst" must be at least 9 bytes long. 
   address will be incremented as needed.
*/
void jg40_getinstruction( word *address, char *inst )
{
	byte opcode = jg40_read4bit( jg40.ROM, *address );

	strcpy( inst, OPCODE_NAMES[opcode] );
	*address = *address + 1;

	/* put the arguments in hexedecimal format */
	if( opcode==OR || opcode==AND )
	{
		byte value = jg40_read4bit( jg40.ROM, *address );
		char str[4];
		*address = *address + 1;
		sprintf( str, " $%01X", value );
		strcat( inst, str );
	} else 
	if( opcode==SED )
	{
		byte value = jg40_read4bit( jg40.ROM, *address );
		char str[5];
		*address = *address + 1;
		sprintf( str, " $%01X", value );
		strcat( inst, str );
	} else
	if( opcode==JZ || opcode==JC || opcode==SES )
	{
		word value = jg40_read12bit( jg40.ROM, *address );
		char str[6];
		*address = *address + 3;
		sprintf( str, " $%03X", value );
		strcat( inst, str );
	}
}



/*
	test code
*/
#ifdef __TESTING__

void cbwrite( byte value )
{
//	printf( "\nwrote %03X to the MMIO port.\n", value );
}

byte cbread()
{
//	printf( "\nreading from MMIO port.\n" );
	return 0;
}

#include "time.h"

int main()
{
	FILE *f;
	byte ROM[ROM_SIZE];
	int file_size;
	int a = TRUE;
	word tmp;
	char inst[10];

	printf( "opening <test.bin>\n" );
	f = fopen( "test.bin", "rb" );
	
	printf( "loading file\n" );
	file_size = fread( &ROM, 1, ROM_SIZE, f );
	
	fclose( f );
	
	printf( "initializing jg40\n" );
	jg40_initialize();
	
	printf( "loading ROM\n" );
	jg40_loadROM( ROM, file_size );

	printf( "setting callbacks for memory mapped IO\n" );
	jg40_setmemorymappedIOcallbacks( cbwrite, cbread );

	printf( "reset and run:\n" );
	jg40_reset();

	while( a==TRUE )
	{
		printf( "%03X (%05d): ", jg40.program_pointer, jg40.program_pointer );
		
		tmp = jg40.program_pointer;
		jg40_getinstruction( &tmp, inst );
		printf( "%s\t", inst );

		a = jg40_execute( 1 );

		/* print the registers */
		printf( "(A:%01X D:%02X T:%02X S:%03X)(", 
				jg40.accumulator,   /* A - 4-bit Accumulator */
	    		jg40.RAM_pointer,   /* D - 8-bit RAM Data Address pointer */
				jg40.SRAM_pointer,  /* T - 6-bit Stack Pointer */
				jg40.ROM_pointer    /* S - 12-bit ROM Data Address pointer */
			  );

		/* print the first 16 bytes of memory */
		for( tmp=0; tmp<16; tmp++ )
		{
			printf( "%02X", jg40.RAM[tmp] );
		}

		printf( ")\n" );
		getch();
	}

	printf("program finished.\n");
	return 0;
}

#endif
