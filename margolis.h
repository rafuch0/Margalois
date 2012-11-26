#ifndef MARGOLIS_H
#define MARGOLIS_H

#define getCellInv(x,y) invSBox[SBox[fixX(x)+fixY(y)*width]] //Define for getting value from InvSBox
#define getCell(x,y) SBox[fixX(x)+fixY(y)*width] //Define for getting vlue from SBox

#define fixX(x) ((width + (x))%width) //Define for Fixing X values (border wrapping)
#define fixY(y) ((height + (y))%height) //Define for Fixing Y values (border wrapping)

#define swapDiag1(x,y) swap(x,y,x+1,y+1) //Defines for calling swap Method for different swaps
#define swapDiag2(x,y) swap(x+1,y,x,y+1)
#define swapHoriz1(x,y) swap(x,y,x+1,y)
#define swapHoriz2(x,y) swap(x,y+1,x+1,y+1)
#define swapVert1(x,y) swap(x,y,x,y+1)
#define swapVert2(x,y) swap(x+1,y,x+1,y+1)

class margolis //Basic functionality Margolis Class for use with SBox and InvSBox (see MargolisSingle)
{
public:
    margolis(); //Default Constructor
    ~margolis(); //Default Destructor
    virtual void setParams ( uint8_t *sboxz, uint8_t *invsboxz, int widthz, int heightz, int threshz ); //Method for setting parameters for normal Margolis
    void doGenerations ( int numGens ); //Method for running N number of generations

protected:
    virtual void swap ( int x1, int y1, int x2, int y2 ); //Method for performing swaps of values in neighborhood
    void doTransition ( int x, int y, uint8_t currState ); //Method for performing the neccessary transition rules for the current configuration
    uint8_t getConfiguration ( int i, int j ); //Method for reporting the current configuration
    void setupRules(); //Method for initializing the default rules

    uint8_t *SBox; //Pointer to SBox (Normal Margolis), or Key (Single Margolis) to be used as the CA Map
    uint8_t *invSBox; //Pointer to InvSBox (Normal Margolis), or NULL (Single Margolis) to be used as the CA Map
    bool rules[16][6]; //Rules Matrix 16 configurations possible for each

    bool evenodd; //Variable for storing current even or odd for Margolis Neighborhood
    int thresh; //Threshhold value for determining if a cell is on or off
    int height; //Height of the CA Map (for 2d indexing into a 1d map)
    int width; //Width of the CA Map (for 2d indexing into a 1d map)
};

class margolisSingle : public margolis //Single Margolis class for use with Key array
{
public:
    void swap ( int x1, int y1, int x2, int y2 ); //Redefined swap Method that doesn't attempt to mix an inverse box
    void setParams ( uint8_t *keyBoxz, int widthz, int heightz, int threshz ); //Method for setting parameters for single Margolis
};

#include "margolis.cpp"

#endif
