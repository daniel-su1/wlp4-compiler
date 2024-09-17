# MIPS Loader with Relocation

## Project Description

This project implements a MIPS assembly loader that can load and execute MERL (MIPS Executable and Relocatable Linkable format) files. The loader supports relocation, allowing programs to be loaded and executed at arbitrary memory addresses.

## Features

- Reads a 32-bit memory address α and a MERL file from standard input
- Loads the MIPS code segment into memory at address α
- Performs relocation on the loaded code
- Executes the loaded code
- Supports self-modifying code
- Prints the loaded code before and after execution

## Implementation Details

The loader performs the following steps:

1. Reads the memory address α and the MERL file header
2. Loads the MIPS code segment into memory at address α, printing each word as it's loaded
3. Reads the footer of the MERL file and performs relocation on the loaded MIPS code
4. Jumps to address α and executes the loaded MIPS code
5. After execution, prints the (potentially modified) loaded MIPS code

## Usage

To use this loader, you need a MIPS simulator that supports the custom `readWord` and `printHex` procedures. The input should be provided in the following format:

1. A 32-bit memory address α
2. The contents of a MERL file

The loader will output:
1. The hexadecimal representation of each word in the code segment as it's loaded
2. The hexadecimal representation of each word in the code segment after execution

## Limitations

- The MERL file footer is assumed to only contain REL entries. ESR and ESD entries are not supported.
- The address α must be between 0x10000 and 0x100000.
- The loader assumes that the loaded code will not modify memory at addresses greater than or equal to the initial value of $30 (stack pointer).

## Example

Given a MERL file containing the following self-modifying code and a load address α of 0x10000:

```mips
beq $0, $0, skip
changeMe: .word 0
skip:
lis $3
.word 241
lis $4
.word changeMe
sw $3, 0($4)
jr $31
```
The loader's output would be:
```
10000001
00000000
00001814
000000F1
00002014
00000010
AC830000
03E00008
10000001
000000F1
00001814
000000F1
00002014
00010004
AC830000
03E00008
```
