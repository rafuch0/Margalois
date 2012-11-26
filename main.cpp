#include <iostream>
#include <math.h>
#include <time.h>

#include "galois.h"

int main ( int argc, char *argv[] )
{
    srand ( time ( 0 ) ); //Initialize the random seed to the time

    galois *newgalois = new galois(); //Create a blocking instance of galois

    system ( "PAUSE" ); //Pause then exit
    return 0;
}
