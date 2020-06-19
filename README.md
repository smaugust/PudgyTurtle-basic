# PudgyTurtle
Source-codes for studying and implementing PudgyTurtle, and
for mounting modified time-memory-data tradeoff attacks.

PudgyTurtle is like a stream-cipher encryption mode: it uses the keystream
to create a variable-length codeword for each nibble (4-bit group) of plaintext,
and then encryptes each codeword by XOR'ing them to a different portion of the
keystream.

SOURCE CODE #1: pt.c
====================
The simplest PudgyTurtle C-language implementation: reads a file and 
encrypts or decrypts it.  Filename is from command-line; output is to stdout.
The secret key is hard-wired into the source code (binary 10101010...1010);
and the keystream generator is hard-wired to be a 24-stage nonlinar feedback 
shift register (NLFSR).  

WARNINGS:
* Very simple, "bare-bones" code for those interested in experimenting
  with and researching PudgyTurtle.
* No input-validation or sanitization is done.
* File I/O, NLFSR, and encryption/decryption routines are not especially efficient.
* No security guarantees are made.
