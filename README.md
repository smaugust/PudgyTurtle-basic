# PudgyTurtle

PudgyTurtle is an "encryption mode" for stream ciphers: it uses some keystream
to encode each plaintext-symbol, and a different part of the keystream to
encipher this codeword (via XOR).  The code is 1-bit error-correcting and 
non-systematic (i.e., the actual plaintext-symbol is not part of the codeword).   

GOALS:
======
  This repository contains a very simple C-language code
  demonstrating basic PudgyTurtle encryption / decryption process.

  This code implements basic, DETERMINISTIC PudgyTurtle, with fixed-size
  input/output: plaintext is grouped into 4-bit symbols (nibbles), and
  ciphertext is produced as 8-bit symbols (bytes).

      * This code is for research purposes only! 
      * It's not a 'secure' production code for end-users. 
      * EG: the secret-key, keystream, and keystream-generator
        state are all purposely exposed to facilitate debugging.

FILES:
======
  LICENSE    (a permissive MIT license for all of this software)
  README.md  (this file)
  pt.c       (ASCII C-language source code)
  pt-linux   (Compiled binary/executable of pt.c)
  test1.in   (A 16-byte [32-nibble] test input containing 0xFFEEDDCC...221100)
  test1.e    (The 32-byte encrypted version of test1.in)
  test2.in   (A 250K test input containing English-language text)
  test2.e    (The 500005-byte encrypted version of test2.in)

SOURCE CODE #1: pt.c
====================
The simplest PudgyTurtle C-language implementation: reads a file and 
encrypts or decrypts it.  Filename is from command-line; output is to stdout.
For simplicity, the keyestream generator and secret key are both "hard-wired" 
into the source code: the secret key is 10101010...1010 in binary;
the keystream generator is a 24-stage nonlinar feedback shift register.

  WARNINGS:
  --------- 
  * Very simple, "bare-bones" code for experimenting with / researching PudgyTurtle;
  * No input-validation or sanitization is included; 
  * File I/O, NLFSR, and encryption/decryption routines could be more efficient;
  * No security guarantees are made (see LICENSE);

  TEST PATTERNS for pt.c
  ----------------------
  Running pt.c on test1.in should produce test1.e. In a Linux environment, this
  these test inputs can be checked with something like this:

    ./pt-linux -ftest1.in >myfile    ... encrypts test1.in and writes result to myfile
    ./pt-linux -fmyfile -d >test1.d  ... decrypts myfile, writing result to test1.d
    diff -s myfile test1.e           ... checks whether encryption is running correctly
    diff -s test1.in test1.d         ... checks for corect decryption; should output:
                                         "Files test1.in and test1.d are identical"  

  Similarly, running pt.c on test2.in should produce test2.e.  The difference 
  between test1.e and test2.e is that test2.e is long enough to demonstrate
  how PudgyTurtle handles "overflow events".  Instead of being 500,000 bytes
  (i.e., exactly twice the length of the input file), test2.e actually 500005 bytes, 
  meaning that 5 overflow events occurred during PudgyTurtle's encoding process. 

# PudgyTurtle
"As is" source-codes for studying and implementing PudgyTurtle, and for mounting a modified time-memory-data tradeoff attack
