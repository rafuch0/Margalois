#ifndef MARGOLIS_CPP
#define MARGOLIS_CPP

#include "margolis.h"

margolis::~margolis()
{} //Unused Default Destructor

margolis::margolis()
{} //Unused Default Constructor

void margolis::setupRules()
{
    /*Setup the BounceGas Rules for the Automata and initialize EvenOdd*/

    evenodd = true;

    for ( int i = 0;i < 16;i++ ) for ( int j = 0;j < 6;j++ ) rules[i][j] = false;
    rules[1][0] = true;
    rules[2][1] = true;
    rules[4][1] = true;
    rules[6][2] = true;
    rules[6][3] = true;
    rules[7][0] = true;
    rules[8][0] = true;
    rules[9][2] = true;
    rules[9][3] = true;
    rules[11][1] = true;
    rules[13][1] = true;
    rules[14][0] = true;
}

void margolis::setParams ( uint8_t *sboxz, uint8_t *invsboxz, int widthz, int heightz, int threshz )
{
    /*Set the Parameters and S-Box pointers for the S-box modifying CA*/

    SBox = sboxz; //Set S-Box Pointer
    invSBox = invsboxz; //Set Inv S-Box Pointer
    width = widthz; //Set Width
    height = heightz; //Set Height
    thresh = threshz; //Set CA Threshold

    setupRules(); //Call Setup Rules
}

void margolisSingle::setParams ( uint8_t *keyBoxz, int widthz, int heightz, int threshz )
{
    /*Set the Parameters and Key Array pointers for the Key modifying CA*/

    SBox = keyBoxz; //Set Key Array pointer
    width = widthz; //Set Width
    height = heightz; //Set Height
    thresh = threshz; //Set CA Threshold

    setupRules(); //Call Setup Rules
}

void margolisSingle::swap ( int x1, int y1, int x2, int y2 )
{
    /*Swap method for Key-Modifying CA*/

    uint8_t temp = getCell ( x1, y1 ); //Swap Key Entries
    getCell ( x1, y1 ) = getCell ( x2, y2 );
    getCell ( x2, y2 ) = temp;
}

void margolis::swap ( int x1, int y1, int x2, int y2 )
{
    /*Swap method for S-Box modifying CA*/

    uint8_t temp = getCellInv ( x1, y1 ); //Swap Inv S-Box Entries
    getCellInv ( x1, y1 ) = getCellInv ( x2, y2 );
    getCellInv ( x2, y2 ) = temp;

    temp = getCell ( x1, y1 ); //Swap S-Box Entries
    getCell ( x1, y1 ) = getCell ( x2, y2 );
    getCell ( x2, y2 ) = temp;
}

void margolis::doTransition ( int x, int y, uint8_t currState )
{
    /*Apply the transition rules to the current parition
    based on its current state*/

    for ( int i = 0;i < 6;i++ ) //Check all Six Swaps
    {
        if ( rules[currState][i] ) //If swap is to be performed
        {
            switch ( i ) //Determine Swap and Do it
            {
            case 0:
                swapDiag1 ( x, y );
                break;
            case 1:
                swapDiag2 ( x, y );
                break;
            case 2:
                swapHoriz1 ( x, y );
                break;
            case 3:
                swapHoriz2 ( x, y );
                break;
            case 4:
                swapVert1 ( x, y );
                break;
            case 5:
                swapVert2 ( x, y );
                break;
            }
        }
    }
}

/*GetConfiguration adds up the the values of the cells in the partition to a number between 0 and 15*/
#define getConfiguration(i,j) ((getCell(i+1,j+1)>thresh?8:0) + (getCell(i,j+1)>thresh?4:0) + (getCell(i+1,j)>thresh?2:0) + (getCell(i,j)>thresh?1:0))

void margolis::doGenerations ( int numGens )
{
    /*Method for performing a number of generations*/

    for ( int k = 0; k < numGens; k++ ) //For all the generations
    {
        evenodd ^= 1; //Invert the EvenOdd Boolean

        for ( int i = evenodd; i < height; i += 2 ) //Start from EvenOdd and count by 2s
        {

            for ( int j = evenodd; j < width; j += 2 ) //Start from EvenOdd and count by 2s
            {
                doTransition ( i, j, getConfiguration ( i, j ) ); //Do the transition for the configuration of the partition
            }
        }
    }
}

#endif
