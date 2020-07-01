/*************************************************
;NAME: pt.c
;AUTHOR: David A August
;DATE: 24 June 2020
; Implements PudgyTurtle encryption / decryption,
; using a simple NLFSR as the keystream generator.
;VERSION: 1.1 
;INPUT (from command-line): 
;  -f<filename>
;  -d<decryption mode> [DEFAULT: encryption]
;OUTPUT: stdout
; *"As-is" code for research purposes (MIT license)
; *SECRET KEY -- pre-set w/in the code to 0xAA...A
; *NLFSR -- pre-set w/in the code 
; *SIMPLE -- no input validation, very basic I/O, etc.
************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <inttypes.h>
//*************************************************
typedef uint64_t U64;
typedef uint8_t U8;
//*************************************************
//NLFSR logic (see nlfsr subroutine below)
#define LINEAR_TAPS    (U64) 0x008303
#define NONLINEAR_TAPS (U64) 0x040080
//*************************************************
//Subroutines
void pudgy_turtle(U64 *, U8 *, int, int); //Encryption/decrpytion routine
U8 nlfsr(U64 *);                          //Updates NLFSR state & returns 1 output bit (as a U8)
U8 get_ksg_nibble(U64 *);                 //Calls nlfsr 4 times & returns 1 nibble of keystream
//*************** MAIN PROGRAM ********************
int main (int argc, char *argv[]) 
{
  U64 i;                      
  char fname[50]={'\0'};  //Name of input file to ENncrypt/DEcrypt
  FILE *fp;               //Pointer to file
  U64 n_bytes;            //Number of bytes input file 
  U8 *data;             //Contents of input file (byte array)
  U64 state;              //Keystream generator (NLFSR) state -- up to 64 bits
  int decrypt = 0;        //DEFAULT mode: encryption 
  int n = 24;             //DEFAULT NLFSR state-size (bits)
  const char *help_message = "\nNAME: pt.c\n\n  Simple PudgyTurtle implementation with NLFSR keystream generator\n\nUSAGE: pt -f<name> [-d]\n\t-f\tName of input file\n\t-d\tDecrypt mode [OPTIONAL: default = encrypt]\n\nEXAMPLE: ./pt -fmydata.in >mydata.enc     -- ENcrypts mydata.in\n\t ./pt -fmydata.enc -d >mydata.dec -- DEcrypts mydata.enc\n\n";


  //Show help message if command-line has no parameters, "-h", or "--help" 
  argc--;                 
  if (! argc || strncmp(argv[1],"-h",2)==0 || strncmp(argv[1],"--help",6)==0) {
    fprintf(stderr, "%s", help_message);
    exit(EXIT_FAILURE);
  }

  //Process command line for filename and encrypt/decrypt flag
  for (i=1; i<=argc; i++) {
    if (strncmp(argv[i],"-f",2) == 0) strncpy(fname, (argv[i]+2), 50);
    else if (strncmp(argv[i],"-d",2) == 0) decrypt = 1;
    else { 
      fprintf(stderr,"\nERROR: Unrecognized command-line parameter [%s]\n\n", argv[i]); 
      exit(EXIT_FAILURE); 
    }
  }

  //Initialize state to n bits of the DEFAULT hard-wired
  //secret key (i.e., the binary 101010...10 pattern)
  state = (U64) 0xAAAAAAAAAAAAAAAA & (((U64) 1 << n) - 1);  
 

  //Open the input file as 'read-only / binary'
  fp = fopen(fname, "rb");   
  if (fp == NULL) {
    fprintf(stderr,"\nERROR: Can not open %s\n\n", fname); 
    exit(1); 
  }

  //Determine n_bytes, the # of bytes in this file
  fseek(fp, 0, SEEK_END);
  n_bytes = ftell(fp);
  rewind(fp);

  //Read the file into "data" array of U8's
  data = (U8 *) malloc(n_bytes * sizeof(U8));  //Allocate memory
  i = 0;                                         
  while (fread(&data[i++], 1, 1, fp)==1) ;     //Read file, byte-by-byte
  fclose(fp);                                  //Close file when done

  //Call PudgyTurtle routine with NLFSR state, data from file, 
  //file-size and encrypt/decrypt mode flag
  pudgy_turtle(&state, data, n_bytes, decrypt);

  //Clean up
  free(data); 
}
//-------------------------------------------------
/**************************************************
;NONLINEAR feedback shift register routine
;SOURCE (for feedback logic):
  Elena Dubrova, "A List of Maximum Period NLFSRs",
  https://eprint.iacr.org/2012/166.pdf
      "n=24: 0,1,8,9,15,(17,18)", which means
  f(x0,x1,x2,...,x23) = x0 + x1 + x8 + x9 + x15 + (x17 * x18)
;
;SOURCE: (for 'Hamming-weight' calculation): 
; www.cs-fundamentals.com/tech-interview/c/c-program-
; to-count-number-of-ones-in-unsigned-integer.php
;
;INPUT:  U64 NLFSR state
;OUTPUT: 0 or 1, returned as a U8 value
*************************************************/
U8 nlfsr(U64 *ksg)
{
    U64 s, s_lin, s_nonlin;
    U64 b1, b2;        

    s = *ksg;  //Temporary copy of current KSG state
   
    //Calculate b1 = x0 + x1 + x8 + x9 + x15 ("linear" taps)
    s_lin = s & LINEAR_TAPS;          //"Linear" portion of NLFSR logic
    for (b1=0; s_lin; s_lin = s_lin & (s_lin-1)) b1++;  
    b1 &= (U64) 1;                    //b1 := XOR of linear taps

    //Calculate b2 = x17 + x18 ("nonlinear" taps)
    s_nonlin = s & NONLINEAR_TAPS;    //"Nonlinear" portion of NLFSR logic
    for (b2=0; s_nonlin; s_nonlin = s_nonlin & (s_nonlin-1)) b2++;  

    //Re-define b2 as 1 only if BOTH nonlinear taps are 1 ("AND" function)
    if (b2 == 2) b2 = 1; else b2 = 0; 

    s = (s) >> 1;            //Shift NLFSR state rightwards 1 position
    s = s | ((b1 ^ b2) << (24 - 1)); //Move feedback bit (b1^b2) to m.s.b. position
    *ksg = s;                //Update the NLFSR state (argument to subroutine)
    return(*ksg & 1);        //Return l.s.b. as the output bit
}  
/**************************************************
;NAME: get_ksg_nibble
; Calls nlfsr routine 4x to return one nibble (four
; bits) as the low-order bits of a U8 value.  If
; the keystream bits are a,b,c,d,e,f,..., then
; the ordering of bits within the nibbles will 
; be dcba, hgfe, ...
;INPUT: U64 KSG state U64
;OUTPUT: 0-15 in the "least signif" bits of a U8
**************************************************/
U8 get_ksg_nibble(U64 *ksg)
{
  U8 newnibble=0;
  newnibble = nlfsr(ksg) << 3;
  newnibble = (newnibble >> 1) | (nlfsr(ksg) << 3);
  newnibble = (newnibble >> 1) | (nlfsr(ksg) << 3);
  newnibble = (newnibble >> 1) | (nlfsr(ksg) << 3);
  return (newnibble);
}
/************************************************
;NAME: PudgyTurtle Encryption / Decryption routines
;DATE: 5/7/2020
;VERSION: 1
;INPUT: state = 64-bit KSG state
;       data = array of plaintext BYTES
;       nbytes = # of plaintext BYTES 
;       decrypt_mode = flag for decrypting
************************************************/
void pudgy_turtle(U64 *state, U8 *data, int nbytes, int decrypt_mode)
{
 int i,j;           //Counters
 U8 lo_nibble;      //low-order (1) or hi-order (0) nibble w/in a byte
 U8 xbyte, xnibble; //Plaintext byte and nibble
 U8 ybyte;          //Ciphertext byte
 U8 knibble;        //Keystream nibble
 U8 mask;           //Mask: 2 keystream nibbles used to encrypt/decrypt a codeword
 U8 hamming;        //Hammming-vector (XOR) btwn xnibble & each keystream nibble
 U8 discrepancy;    //Discrepancy code: 0 (exact match) or {1,2,3,4} (1-bit mismatch)
 U8 fails;          //Failure-counter: how many keystream nibbles do NOT match xnibble
 U8 codeword;       //Codeword: failure-counter [5 bits] || discrepancy-code [3 bits]
 int match;         //(0/1)-flag: does xnibble match current keystream nibble?
 //valid_hammings: "1" --> exact match or 1-bit mismatch btwn two 4-bit patterns
 int valid_hammings[16] = {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0};  
 //dcodes: discrepancy codes for ENcryption 
 U8 dcodes[9] = {0, 1, 2, 0, 3, 0, 0, 0, 4};  
 //inverse_dcodes: "inverse" discrepancy codes for DEcryption 
 U8 inverse_dcodes[5] = {0, 1, 2, 4, 8};      

 if (! decrypt_mode) {                  
  //For ENcryption...
  for (i=0; i<2*nbytes; i++) {    //i: Plaintext nibble index

   //Current plaintext BYTE
   if (i%2==0) xbyte = *(U8 *) data++;  

   //Strip off high- or low-order nibble from current plaintext byte 
   xnibble = (i % 2 == 0) ? ((xbyte & 0xF0) >> 4) : xbyte & 0x0F; 

   //Mask: the next two available keystream nibbles
   mask = (get_ksg_nibble(state) << 4) | get_ksg_nibble(state);  

   fails = 0;    //Initialize failure-counter to 0
   do {          //Repeat this code-block until a match is found

    //Binary difference between xnibble & each new keystream nibble
    hamming = xnibble ^ get_ksg_nibble(state);  
   
    match = valid_hammings[hamming]; //0/1-flag for match failure/success 
    if (! match) fails += 1;         //If NO match, increment failure-counter

    if (fails == 32) {       
     //OVERFLOW EVENT: 32 failures without a match
     ybyte = 0xFF ^ mask;  //...Set ciphertext byte to 0xFF XOR mask,
     printf("%c", ybyte);  //...Display this byte, make a new mask,
     mask = (get_ksg_nibble(state) << 4) | get_ksg_nibble(state);   
     fails = 0;            //...And reset failure-counter to 0
    } 
   } while (! match);

   //If a matching keystream nibble HAS been found...
   discrepancy = dcodes[hamming]; 

   //Construct the codeword 
   codeword = fails << 3;    //Upper 5 bits: failure-counter 
   codeword |= discrepancy;  //Lower 3 bits: discrepancy-code

   ybyte = codeword ^ mask;  //ENCRYPT the codeword with the mask
   printf("%c", ybyte);      //Display ciphertext byte
  }                          //Loop through plaintext nibbles

 } else {  
  //For DEcryption....

  lo_nibble=0;                //1st DEcrypted nibble is the hi-order one
  for (i=0; i<nbytes; i++) {  //i: index of each ciphertext BYTE

   ybyte = *(U8 *) data++;    //Get ciphertext byte

   //Create a mask from next 2 keystream nibbles
   mask = (get_ksg_nibble(state) << 4) | get_ksg_nibble(state);

   //Unmask the ciphertext byte to re-create the codeword
   codeword = ybyte ^ mask;

   if (codeword == 0xFF) {  
    //If the codeword is 0xFF (overflow event), then
    //generate and discard 32 keystream nibbles
    for (j=0; j<32; j++) knibble = get_ksg_nibble(state);

   } else {  

    //If no overflow, split the codeword into its components:
    fails = (codeword & 0xF8) >> 3;   //Failure-counter = first 5 bits 
    discrepancy = (codeword & 0x07);  //Discrepancy-code = last 3 bits 

    //Generate "fails+1" more keystream nibbles to reach 
    //the one that matches the original plaintext nibble.
    for (j=0; j<=fails; j++) knibble = get_ksg_nibble(state); 

    //Adjust this nibble via the inverse discrepancy code
    xnibble = knibble ^ inverse_dcodes[discrepancy];   

    //Make a plaintext BYTE from each pair of decrypted nibbles
    xbyte = (lo_nibble == 0) ? xnibble << 4 : xbyte | xnibble;  

    if (lo_nibble == 1) printf("%c", xbyte);  //Print decrypted BYTE if complete
    lo_nibble = !lo_nibble;                   //Flip lo_nibble between 0 and 1

   } //else if no overflow
  }  //for (i=0; i<nbytes...
 }   //If (!decrypt_mode)...
}
