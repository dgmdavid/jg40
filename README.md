# JG40

JG40 is a virtual/hypothetical portable 4-bit video-game system that I made a while ago.

It contains a simple assembler for the CPU and an emulator for the system.

Here's a quickref of the system:
```
 --------------------------------------------------------------------------------
 - Jaguar Virtual 4-bit Hand-held Video-Game System
 - by David G. Maziero
 -
 - JG40 Quick Reference
 --------------------------------------------------------------------------------

  Instruction set
  ---------------
  ____________________________________________________________________________
 | Mnemonic |Opc | CZ | Description                                  | N | Cc |
 |----------|----|----|----------------------------------------------|---|----|
 | LDA      | $0 | -Z | Load A with memory at D.                     | 1 |  2 |
 | STA      | $1 | -- | Store A in memory at D.                      | 1 |  2 |
 | ADD      | $2 | CZ | Add memory value in D to A.                  | 1 |  4 |
 | OR i4    | $3 | -Z | Logical OR with A.                           | 2 |  3 |
 | AND i4   | $4 | -Z | Logical AND with A.                          | 2 |  3 |
 | NOT      | $5 | -Z | Invert all bits of A.                        | 1 |  2 |
 | SHL      | $6 | CZ | Binary left shift in A.                      | 1 |  2 |
 | SHR      | $7 | CZ | Binary right shift in A.                     | 1 |  2 |
 | PUA      | $8 | -- | Push A to stack.                             | 1 |  4 |
 | POA      | $9 | -Z | Pop A from stack.                            | 1 |  4 |
 | CMP      | $A | CZ | Compare A with memory in D.                  | 1 |  4 |
 | JZ a12   | $B | -- | Conditional jump if Zero flag is set.        | 4 |  8 |
 | JC a12   | $C | -- | Conditional jump if Carry flag is set.       | 4 |  8 |
 | SED i4   | $D | -- | Set D.                                       | 2 |  5 |
 | SES i12  | $E | -- | Set S.                                       | 4 |  7 |
 | TRR      | $F | -Z | Transfer value in S to RAM in D.             | 1 |  6 |
  ----------------------------------------------------------------------------
                     ______________________________________
       Conventions: | i4  | Immediate 4-bit value          | 
                    | i12 | Immediate 12-bit value         |
                    | a12 | Immediate 12-bit address       |
                    | A   | Register A (accumulator)       |
                    | C   | Carry flag is affected         |
                    | Z   | Zero flag is affected          |
                    | Cc  | Cycles taken by an instruction |
                    | N   | Nibbles used by an instruction |
                     --------------------------------------
                     
  Memory Mapped I/O Port $F function list
  ---------------------------------------
    ________________________________________________________________________
   | write to $F   | next action              | result                      |
   |---------------|--------------------------|-----------------------------|
   |  $0           | write 2 nibbles          | Unset a pixel in screen     |
   |  $1           | write 2 nibbles          | Set a pixel in screen       |
   |  $2           | write 2 nibbles and read | See if a pixel is set       |
   |  $3           | write 64 nibbles         | Fill the screen             |
   |  $4           | nothing                  | CPU will wait for the vsync |
   |  $5           | nothing                  | Clear the entire screen     |
   |  $6           | write 1 nibble           | Perform a screen scrolling  |
   |  $7           | nothing                  | Negate each pixel           |
   |---------------|--------------------------|-----------------------------|
   |  $A           | write up to 17 nibbles   | Program a sequence of tones |
   |  $B           | nothing                  | Play/Replay the last music  |
   |  $C           | read a nibble            | Check if music is playing   |
   |  $D           | nothing                  | Stop the current music      |
   |  $E           | nothing                  | CPU'll wait music to finish |
   |---------------|--------------------------|-----------------------------|
   |  $F           | read a nibble            | Get buttons state (input)   |
    ------------------------------------------------------------------------

EOF
```
