#ifndef TESTS_H
#define TESTS_H

#include <iostream>
#include <fstream>
#include <cassert>
#include <sys/stat.h>
#include <math.h>

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::cin;
using std::ifstream;
using std::ofstream;
using std::ios;

#define BIT(n) (1 << (n))

class tests //Class for performing various analysis
{

public:
    tests ( uint8_t *sboxz, uint8_t *invsboxz ); //Constructor takes an SBox and invSBox pointer
    ~tests(); //Default Destructor

    void testInvertibility(); //Verifies invertibility between SBox and invSBox
    void testNonLinearity ( bool verbose ); //Reports the Non-Linearity measures of the SBox
    void testBitChanges ( bool verbose ); //Counts the number of bit changes between sbox inputs and their outputs
    void BitARFWT(); //Performs Walsh Hadamard Transform for Non-Linearity Measure
    void testDataHistogram ( bool verbose ); //Creates a data-histogram file based on an input file
    void testMidPoints ( bool verbose ); //Reports the MidPoints of the On and Off values in the CA SBox
    void testDistanceToCenter ( bool verbose ); //Reports the Distance to the Center of MidPoints of the On and Off values in the CA SBox
    void testAvalanche ( bool verbose ); //Reports the effects of modifying single bits in SBox input data
    void testAll ( bool verbose ); //Performs all tests
    void testEntropy ( bool verbose ); //Reports the Entropy of two input files and their conditional entropy
    void printSbox(); //Prints out the SBox

private:
    uint8_t *sbox; //Pointer to SBox
    uint8_t *invsbox; //Pointer to InvSBox

    int bits[256]; //Walsh-Hadamard Transform Bits Array
};

#include "tests.cpp"

#endif
