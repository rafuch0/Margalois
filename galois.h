#ifndef GALOIS_H
#define GALOIS_H

#include <iostream>
#include <fstream>
#include <cassert>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "margolis.h"
#include "tests.h"

using std::cout;
using std::endl;
using std::ios;
using std::hex;
using std::dec;
using std::ifstream;
using std::ofstream;
using std::cin;
using std::fill;

const uint8_t generators[] =
    { /* The Galois Generator Table */
        0x03, 0x05, 0x06, 0x09, 0x0b, 0x0e, 0x11, 0x12, 0x13, 0x14, 0x17, 0x18, 0x19, 0x1a, 0x1c, 0x1e,
        0x1f, 0x21, 0x22, 0x23, 0x27, 0x28, 0x2a, 0x2c, 0x30, 0x31, 0x3c, 0x3e, 0x3f, 0x41, 0x45, 0x46,
        0x47, 0x48, 0x49, 0x4b, 0x4c, 0x4e, 0x4f, 0x52, 0x54, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5f,
        0x64, 0x65, 0x68, 0x69, 0x6d, 0x6e, 0x70, 0x71, 0x76, 0x77, 0x79, 0x7a, 0x7b, 0x7e, 0x81, 0x84,
        0x86, 0x87, 0x88, 0x8a, 0x8e, 0x8f, 0x90, 0x93, 0x95, 0x96, 0x98, 0x99, 0x9b, 0x9d, 0xa0, 0xa4,
        0xa5, 0xa6, 0xa7, 0xa9, 0xaa, 0xac, 0xad, 0xb2, 0xb4, 0xb7, 0xb8, 0xb9, 0xba, 0xbe, 0xbf, 0xc0,
        0xc1, 0xc4, 0xc8, 0xc9, 0xce, 0xcf, 0xd0, 0xd6, 0xd7, 0xda, 0xdc, 0xdd, 0xde, 0xe2, 0xe3, 0xe5,
        0xe6, 0xe7, 0xe9, 0xea, 0xeb, 0xee, 0xf0, 0xf1, 0xf4, 0xf5, 0xf6, 0xf8, 0xfb, 0xfd, 0xfe, 0xff
    };

class galois
{
public:
    galois(); //Default constructor
    ~galois(); //Default Destructor

private:
    uint8_t gmul ( uint8_t a, uint8_t b ); //Function for multiplying two numbers without lookup table
    uint8_t gmulLookup ( uint8_t a, uint8_t b ); //Function for multiplying two numbers with lookup table
    uint8_t gmulInverse ( uint8_t in ); //Function for calculating the multiplicative inverse of a number
    uint8_t sub ( uint8_t in ); //Function for substituting a byte with the sbox
    uint8_t rcon ( uint8_t in ); //Function for exponentiating 2^in used in key expansion
    uint8_t ROTL ( uint8_t x, uint8_t s ); //Function for left circular shifting an 8bit number s times
    uint8_t ROTR ( uint8_t x, uint8_t s ); //Function for right circular shifting an 8bit number s times

    void gmixColumn ( uint8_t *r ); //Rijndaels MixColumn step
    void invGmixColumn ( uint8_t *r ); //Rijndaels Inverse MixColumn step
    void rotateStateL ( int offset ); //RotateStateLeft, takes offset into one dimensional state variable
    void rotateStateR ( int offset ); //RotateStateRight, takes offset into one dimensional state variable
    void shiftRows(); //Calls RotateStateL to perform the neccessary shifts for 256-bit blocks
    void invShiftRows(); //Calls RotateStateR to perform the neccessary unshifts for 256-bit blocks
    void mixColumns(); //Calls gMixColumn to perform mixing on each column
    void invMixColumns(); //Calls invGmixColumn to perform unmixing on each column
    void substituteBytes(); //Substitutes bytes with their SBox lookup
    void invSubstituteBytes(); //UnSubstitutes bytes with their invSBox lookup
    void addRoundKey ( int i ); //Adds (xors) the indicated roundkey to the state

    void textUserPrompt(); //Prompts the user for required input
    void initCipher(); //Initializes the core functionality of the cipher
    void initIV(); //Initializes the IV for CBC Mode
    void doEncryptDecrypt(); //Calls encrypt or decrypt function based on user input
    void generateLogTables(); //Generates the Logarithm tables for the Galois Field
    void generateGmulInverse(); //Generates the gmulInverse table
    void generateSBoxes(); //Generates the SBox and InvSBox
    void schedule_core ( uint8_t *in, uint8_t i ); //Function used in key expansion
    void rotate ( uint8_t *in ); //Rotate function used in key expansion
    void expand_key(); //Key Expansion function
    void doRounds(); //Rijndaels doRounds function
    void doInvRounds(); //Rijndaels doInvRounds function
    void encrypt(); //Encryption function with File IO and Padding
    void decrypt(); //Decryption function with File IO and DePadding
    void doCBCXOR(); //CBC function to XOR over the state
    void doCBCUpdate ( uint8_t source[32], uint8_t dest[32] ); //CBC function for updating the IV
    void padState ( int padVal ); //Function for Padding the State
    void padFullState ( int padVal ); //Function for adding an entire pad block

    uint8_t hexToInt ( char first, char second ); //Function for converting user hex key input into real hex numbers
    bool verifyGenerators(); //Function for verifying the static generator table

    uint8_t ltable[256]; //Log Table
    uint8_t etable[256]; //Exponentiation Table
    uint8_t gmulInv[256]; //Multiplicative Inverse Table
    uint8_t SBox[256]; //SBox
    uint8_t SBoxInv[256]; //Inverse SBox
    uint8_t key[480]; //The Key and expanded key space

    uint8_t state[32]; //The state, or current block
    uint8_t IV[32]; //The initialization vector
    uint8_t IVTemp[32]; //Space for a temporary IV
    int rounds; //Number of rounds in the encryption

    bool encryptBool; //Boolean from user input
    bool runGensBeforeKey; //CA Key expansion modifying boolean from user input
    int galoisGenSeed; //Galois Generator seed from user input
    int generationsPerBlock; //CA Generations from user input
    int keyGenerationsPerBlock; //KeyCA Generations from user input
    int threshCA; //CA Threshold from user input
    char ciphFile[33]; //Filename for ciphertext
    char textFile[33]; //Filename for plaintext
    char userKey[32]; //User input key space

    margolis *theCA; //The SBox and InvSBox Modifying CA
    margolisSingle *theKeyCA; //The Key modifying CA
    tests *theTest; //Object for the tests class
};

#include "galois.cpp"

#endif

