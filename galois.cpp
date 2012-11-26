#ifndef GALOIS_CPP
#define GALOIS_CPP

#include "galois.h"

galois::galois()
{
    /*Default Constructor*/
    verifyGenerators(); //Verify the Generator Tables

    /*Blank all arrays*/
    fill ( key, key + 480, 0 );
    fill ( ltable, ltable + 256, 0 );
    fill ( etable, etable + 256, 0 );
    fill ( gmulInv, gmulInv + 256, 0 );
    fill ( SBox, SBox + 256, 0 );
    fill ( SBoxInv, SBoxInv + 256, 0 );
    fill ( state, state + 32, 0 );
    fill ( IV, IV + 32, 0 );
    fill ( IVTemp, IVTemp + 32, 0 );
    fill ( userKey, userKey + 32, 0 );
    fill ( ciphFile, ciphFile + 33, 0 );
    fill ( textFile, textFile + 33, 0 );

    textUserPrompt(); //Prompt for User Parameters

    initCipher();  //Initialize the cipher
    doEncryptDecrypt(); //Perform User Function
}

void galois::initCipher()
{
    rounds = 14;  //Set Default Rounds
    generateLogTables(); //Generate the Log Tables
    generateGmulInverse(); //Generate the Multiplicative Inverse Tables
    generateSBoxes();  //Generate the S-Box and Inverse S-Box
    theCA = new margolis(); //Create the S-Box CA instance
    theCA->setParams ( SBox, SBoxInv, 16, 16, threshCA ); //Set the user chosen parameters

    theTest = new tests ( SBox, SBoxInv ); //Create the Tests class

    if ( runGensBeforeKey ) //If user chose to modify key expansion
    {
        theCA->doGenerations ( generationsPerBlock ); //Modify the S-Box during initialization
    }

    expand_key(); //Expand the Users Key

    theKeyCA = new margolisSingle(); //Create the Key CA instance
    theKeyCA->setParams ( key, 24, 20, threshCA ); //Set the user chosen parameters
}

uint8_t galois::hexToInt ( char first, char second )
{
    /*Method for converting ascii hex characters to numbers*/
    char hex[3];
    char *stop;
    hex[0] = first;
    hex[1] = second;
    hex[2] = 0;
    return strtol ( hex, &stop, 16 );
}

void galois::textUserPrompt()
{
    /*Prompt the User for their options*/
    char input[129];
    fill ( input, input + 129, 0 );

    char choice = 0;

    do
    {
        cout << "Encrypt or Decrypt? (e,d) " << endl;
        choice = cin.get();
    }

    while ( ( choice != 'e' ) && ( choice != 'd' ) );

    cin.ignore();

    encryptBool = ( choice == 'e' );

    do
    {
        cout << "Would you like to enter a 1)Passphrase or 2)Key? " << endl;
        choice = cin.get();
    }

    while ( ( choice != '1' ) && ( choice != '2' ) );

    cin.ignore();

    if ( choice == '1' )
    {
        cout << "***Input 32 Ascii Characters****" << endl;
        cin.getline ( input, 33 );

        for ( int i = 0;i < 32;i++ )
        {
            userKey[i] = input[i];
        }
    }

    else
    {
        do
        {
            cout << "***********************Input 64 Hex Digit Key*******************" << endl;
            cin.getline ( input, 65 );
        }

        while ( cin.gcount() != 65 );

        for ( int i = 0;i < 64;i += 2 )
        {
            userKey[i/2] = hexToInt ( input[i], input[i+1] );
        }
    }

    do
    {
        cout << "***TextFile Name: <=32 Chars****" << endl;
        cin.getline ( textFile, 33 );
    }

    while ( cin.gcount() == 1 );

    do
    {
        cout << "***CiphFile Name: <=32 Chars****" << endl;
        cin.getline ( ciphFile, 33 );
    }

    while ( cin.gcount() == 1 );

    do
    {
        cout << "Enter your galois generator number (0-127)" << endl;
        cin >> galoisGenSeed;
    }

    while ( ( galoisGenSeed > 127 ) || ( galoisGenSeed < 0 ) );

    cin.ignore();

    galoisGenSeed = generators[galoisGenSeed];

    do
    {
        cout << "Enter the number of generations per block: " << endl;
        cin >> generationsPerBlock;
        cin.ignore();
    }

    while ( ( generationsPerBlock < 0 ) || ( generationsPerBlock > 128 ) );

    do
    {
        cout << "Enter the number of key generations per block: " << endl;
        cin >> keyGenerationsPerBlock;
        cin.ignore();
    }

    while ( ( keyGenerationsPerBlock < 0 ) || ( keyGenerationsPerBlock > 128 ) );

    if ( generationsPerBlock > 0 ) do
        {
            cout << "Enter the threshold value for the Automata: (64-192) (127 RECOMMENDED) " << endl;
            cin >> threshCA;
            cin.ignore();
        }

        while ( ( threshCA <= 63 ) || ( threshCA >= 193 ) );

    choice = 'n';

    if ( generationsPerBlock > 0 ) do
        {
            cout << "Would you like to run (" << dec << generationsPerBlock << ") generations before key expansion? (y,n) " << endl;
            choice = cin.get();
            cin.ignore();
        }

        while ( ( choice != 'y' ) && ( choice != 'n' ) );

    runGensBeforeKey = ( choice == 'y' );
}

galois::~galois()
{
    /*Default Destructor, Delete instantiated objects*/
    delete theCA;
    delete theKeyCA;
    delete theTest;
}

void galois::initIV()
{
    /*CBC Initialization Vector initilization Method*/
    clock_t start_tick; //Create clock object

    for ( int x = 0;x < 32;x++ ) //For the size of our block
    {
        state[x] = 0; //Blank Entry
        start_tick = clock(); //Set clocks current time

        while ( clock() == start_tick ) //While still current time
        {
            state[x]++; //Increment this entry in the block
        }
    }

    doRounds(); //Encrypt the state with the users key
    doCBCUpdate ( state, IV ); //Update the IV

    for ( int x = 0;x < 32;x++ ) //For the size of our block
    {
        state[x] = 0; //Blank Entry
        start_tick = clock(); //Set clocks current time

        while ( clock() == start_tick ) //While still current time
        {
            state[x]++; //Increment this entry in the block
        }
    }

    doCBCXOR(); //CBC XOR with our first IV
    doRounds(); //Encrypt the IV with users key

    doCBCUpdate ( state, IV ); //Update the FINAL IV
}

void galois::doEncryptDecrypt()
{
    /*Method for simply calling Encrypt or Decrypt based on user input*/
    if ( encryptBool ) encrypt();
    else decrypt();
}

void galois::encrypt()
{
    /*The Encryption Method*/
    initIV(); //Initialize the IV

    struct stat results; //Get the FileSize
    int filesize = stat ( textFile, &results );
    filesize = results.st_size;

    ifstream text ( textFile, ios::in | ios::binary ); //Open the Input and Output Files
    ofstream ciph ( ciphFile, ios::out | ios::binary );

    assert ( text ); //Assert that the files were opened
    assert ( ciph );

    if ( filesize == 0 ) //Refused to Encrypt an Empty File
    {
        cout << "Input file size is zero! Not Encrypting " << endl;
        exit ( 0 );
    }

    ciph.write ( ( char* ) IV, sizeof ( IV ) ); //Write the IV as the first block of the ciphertext

    for ( int z = 0;z <= filesize;z += 32 ) //For all blocks in the file
    {
        text.read ( ( char* ) state, sizeof ( state ) ); //Read a block of text

        if ( z + 32 > filesize ) //If last block
        {
            int padVal = 32 - ( filesize % 32 ); //Calculate the number of bytes needed to fill the last block

            if ( padVal != 32 ) //If the file didn't end as a perfect multiple of the block size (32)
            {
                padState ( padVal ); //Pad the Last Block to fill it out
                doCBCXOR(); //Do CBC XOR as usual
                doRounds(); //Do Encryption rounds as Usual
                theCA->doGenerations ( generationsPerBlock ); //Run the CA on the SBox as usual
                theKeyCA->doGenerations ( keyGenerationsPerBlock ); //Run the CA on the Key as usual
                doCBCUpdate ( state, IV ); //Update the IV with encrypted block
                ciph.write ( ( char* ) state, sizeof ( state ) ); //Output the last block
                padFullState ( padVal ); //Prepare the final padding block
            }
            else
            {
                padFullState ( 0 ); //Only adding the final padding block because file was a multiple of block size
            }
        }

        doCBCXOR(); //Either doing normal processing or finishing final padding block
        doRounds(); //Do Encryption rounds
        theCA->doGenerations ( generationsPerBlock ); //Do S-Box Generations
        theKeyCA->doGenerations ( keyGenerationsPerBlock ); //Do Key Generations
        doCBCUpdate ( state, IV ); //Update the IV with the encrypted block

        ciph.write ( ( char* ) state, sizeof ( state ) ); //Output the block
    }

    text.close(); //Close the files
    ciph.close();
}

void galois::padState ( int padVal )
{
    /*Method to pad a block to fill it up with padVal + 32*/
    for ( int i = 31;i >= 32 - padVal;i-- ) //Starting from the end, write as many as needed
    {
        state[i] = padVal + 32; //Fill with 32 + Count
    }
}

void galois::padFullState ( int padVal )
{
    /*Method to pad a full block with 32 + Count*/
    for ( int i = 31;i >= 0;i-- )
    {
        state[i] = padVal + 32; //PadVal will be zero if the file was a multiple of 32
    }
}

void galois::decrypt()
{
    /*Method for encrypting a file*/

    struct stat results; //Get the FileSize
    int filesize = stat ( ciphFile, &results );
    filesize = results.st_size - 64; //Subtract the IV and Final Padding block from the calculations

    ifstream ciph ( ciphFile, ios::in | ios::binary ); //Open the files
    ofstream text ( textFile, ios::out | ios::binary );

    assert ( ciph ); //Verify the files are open
    assert ( text );

    ciph.read ( ( char* ) IV, sizeof ( IV ) ); //Read the IV from the first block of the ciphertext

    for ( int z = 0;z < filesize;z += 32 ) //For all blocks in the file
    {
        ciph.read ( ( char* ) state, sizeof ( state ) ); //Read in an ciphertext block

        doCBCUpdate ( state, IVTemp ); //Backup this block as the next IV
        doInvRounds(); //Do the Inverse Rounds
        theCA->doGenerations ( generationsPerBlock ); //Do S-Box CA Generations
        theKeyCA->doGenerations ( keyGenerationsPerBlock ); //Do Key CA Generations
        doCBCXOR(); //Do CBC XOR
        doCBCUpdate ( IVTemp, IV ); //Update the IV with the temp IV

        if ( z + 32 >= filesize ) //If Final Block
        {
            uint8_t finalState[32]; //Create a holding space for the final state
            memcpy ( finalState, state, 32 ); //Copy the state into finalState

            ciph.read ( ( char* ) state, sizeof ( state ) ); //Read the Final Padding Block
            doCBCUpdate ( state, IVTemp ); //Update the IVTemp with the state
            doInvRounds(); //Do the Inverse Rounds
            theCA->doGenerations ( generationsPerBlock ); //Do S-Box CA Generations
            theKeyCA->doGenerations ( keyGenerationsPerBlock ); //Do Key CA Generations
            doCBCXOR(); //Do CBC XOR

            if ( state[0] == 32 ) //If the final padding block's last entry is 32
            {
                text.write ( ( char* ) finalState, 32 ); //Write the ENTIRE final state out, original filesize % 32 was 0
            }
            else
            {
                text.write ( ( char* ) finalState, 32 - ( state[0] % 32 ) ); //Otherwise write out the number of bytes that weren't padding
            }
        }
        else
        {
            text.write ( ( char* ) state, sizeof ( state ) ); //Write out a normal block of decrypted plain-text, wasnt last block.
        }
    }

    text.close(); //Close files
    ciph.close();
}

void galois::doCBCXOR()
{
    /*Do the XOR operation of CBC Mode*/
    for ( int i = 0;i < 32;i++ ) state[i] ^= IV[i]; //XOR each state entry with the IV
}

void galois::doCBCUpdate ( uint8_t source[32], uint8_t dest[32] )
{
    /*Wrapper Method for copying to and from state, IV, IVTemp*/
    memcpy ( dest, source, 32 );
}

void galois::doRounds()
{
    /*Rijndael DoRounds*/
    addRoundKey ( 0 ); //Add Key 0

    for ( int i = 1;i < rounds;i++ ) //For all rounds
    {
        substituteBytes(); //S-Box
        shiftRows(); //Shift
        mixColumns(); //Mix
        addRoundKey ( i ); //Add Key i
    }

    substituteBytes(); //Final Sbu
    shiftRows(); //Final Shift
    addRoundKey ( 14 ); //Final Key
}

void galois::doInvRounds()
{
    addRoundKey ( 14 ); //Add Key 14
    invShiftRows(); //UnShift

    for ( int i = rounds - 1;i > 0;i-- ) //For all Rounds in reverse
    {
        invSubstituteBytes(); //Inv S-Box
        addRoundKey ( i ); //Add Key i
        invMixColumns(); //UnMix
        invShiftRows(); //UnShift
    }

    invSubstituteBytes(); //Inv S-Box

    addRoundKey ( 0 ); //Add Key 0
}

void galois::substituteBytes()
{
    /*Substitute all bytes in state with their S-Box entries*/
    for ( int i = 0;i < 32;i++ )
    {
        state[i] = SBox[state[i]];
    }
}

void galois::invSubstituteBytes()
{
    /*Substitute all bytes in the state with their Inverse S-Box entries*/
    for ( int i = 0;i < 32;i++ )
    {
        state[i] = SBoxInv[state[i]];
    }
}

void galois::addRoundKey ( int round )
{
    /*XOR Key block with state block*/
    int location = 32 * round; //Calculate the offset in the expanded key

    for ( int i = 0;i < 32;i++ ) //For all entries in the key block
    {
        state[i] ^= key[location++]; //Do XOR (addition)
    }
}

uint8_t galois::ROTL ( uint8_t x, uint8_t s )
{
    /*Circular rotate a byte left by s bits*/
    return ( uint8_t ) ( ( ( x ) << ( s& ( 8 - 1 ) ) ) | ( ( x ) >> ( 8 - ( s& ( 8 - 1 ) ) ) ) );
}

uint8_t galois::ROTR ( uint8_t x, uint8_t s )
{
    /*Circular rotate a byte right by s bits*/
    return ( uint8_t ) ( ( ( x ) >> ( s& ( 8 - 1 ) ) ) | ( ( x ) << ( 8 - ( s& ( 8 - 1 ) ) ) ) );
}

bool galois::verifyGenerators()
{
    /*Method to algorithmically verfiy the generator tables*/
    uint8_t check = 0xb7; //Default Check Value

    for ( int c = 0; c < 128; c++ ) //For all entries in the generator table
    {
        check ^= ROTL ( generators[c], ROTR ( generators[127-c], c ) ); //Do some Mixing
    }

    uint8_t compare = 1; //Algorithmically build compare

    compare = ROTL ( compare, 2 );
    compare |= ROTL ( compare, 2 );
    compare |= ROTR ( compare, 6 );

    if ( check != compare ) //If not equal, tables modified
    {
        cout << "Error! Generator table has been altered! Check: " << hex << ( uint16_t ) check << " Compare: " << hex << ( uint16_t ) compare << endl;
    }

    else
    {
        cout << "Generator Tables Verified! Check: " << hex << ( uint16_t ) check << " Compare: " << hex << ( uint16_t ) compare << endl;
    }
}

uint8_t galois::gmul ( uint8_t a, uint8_t b )
{
    /*Perform multiplication in the galois field*/
    uint8_t p = 0;
    uint8_t hi_bit_set;

    for ( int counter = 0; counter < 8; counter++ )
    {
        if ( ( b & 1 ) == 1 )
            p ^= a;

        hi_bit_set = ( a & 0x80 );

        a <<= 1;

        if ( hi_bit_set == 0x80 )
            a ^= 0x1b;

        b >>= 1;
    }

    return p;
}

void galois::generateLogTables()
{
    /*Generate Log Tables with the given Generator Seed*/
    etable[0] = 1;
    etable[255] = 1;

    for ( int c = 1;c < 256;c++ )
    {
        etable[c] = gmul ( etable[c-1], galoisGenSeed );
        ltable[etable[c]] = c;
    }
}

/*Very fast define Method for performing multiplications with the lookup tables*/
#define gmulLookupDefine(a, b) ((a==0)?0:etable[(ltable[a]+ltable[b])%255])

uint8_t galois::gmulLookup ( uint8_t a, uint8_t b )
{
    /*Slow Method for performing multiplication with the lookup tables*/
    return ( a == 0 ) ? 0 : etable[ ( ltable[a] + ltable[b] ) % 255];
}

uint8_t galois::gmulInverse ( uint8_t in )
{
    /*Method for calculating the multiplicative inverse from the lookup tables*/
    if ( in == 0 ) return 0;
    else return etable[ ( 255 - ltable[in] ) ];
}

void galois::generateGmulInverse()
{
    /*Method for generating the gmulInverse table*/
    gmulInv[0] = 0;

    for ( int c = 1;c < 256;c++ ) //For all 256 divisors
    {
        gmulInv[c] = gmulInverse ( c ); //Populate the gmulInv table
    }
}

uint8_t galois::sub ( uint8_t in )
{
    /*Method for generating the S-box entries based on the Generator Seed*/
    uint8_t s, x;
    s = x = gmulInv[in];

    for ( int c = 0; c < 4; c++ )
    {
        s = ( s << 1 ) | ( s >> 7 );
        x ^= s;
    }

    x ^= galoisGenSeed;

    return x;
}

void galois::generateSBoxes()
{
    /*Method for generating the S-Box tables using sub*/
    for ( int c = 0;c < 256;c++ ) //For each byte value 0 - 255
    {
        SBox[c] = sub ( c ); //Populate the S-Box with its sub value
        SBoxInv[SBox[c]] = c; //Populate the SBoxInverse location of the sub value with the byte value
    }
}

void galois::mixColumns()
{
    /*Method for performing gmixColumn on all columns of the state*/
    uint8_t temp[4];

    for ( int j = 0;j < 8;j++ ) //For all 8 columns
    {
        for ( int i = 0;i < 4;i++ ) //For all 4 entries in a column
        {
            temp[i] = state[i*8+j]; //Copy them from the state to temp
        }

        gmixColumn ( temp ); //Mix temp

        for ( int i = 0;i < 4;i++ ) //For all 4 entries in a column
        {
            state[i*8+j] = temp[i]; //Restore them into the state from temp
        }
    }
}

void galois::invMixColumns()
{
    /*Method for performing invGmixColumn on all columns of the state*/
    uint8_t temp[4];

    for ( int j = 0;j < 8;j++ ) //For all 8 columns
    {
        for ( int i = 0;i < 4;i++ ) //For all 4 entries in a column
        {
            temp[i] = state[i*8+j]; //Copy them from the state to temp
        }

        invGmixColumn ( temp ); //UnMix temp

        for ( int i = 0;i < 4;i++ ) //For all 4 entries in a column
        {
            state[i*8+j] = temp[i]; //Restore them into the state from temp
        }
    }
}

void galois::gmixColumn ( uint8_t *r )
{
    /*Perform mixColumn on a single column*/

    uint8_t a[4]; //Make holding location
    memcpy ( a, r, 4 ); //Copy original into holding

//Perform matrix multiplication by {{2,3,1,1}{1,2,3,1}{1,1,2,3}{3,1,1,2}}
    r[0] = gmulLookupDefine ( a[0], 2 ) ^ gmulLookupDefine ( a[3], 1 ) ^ gmulLookupDefine ( a[2], 1 ) ^ gmulLookupDefine ( a[1], 3 );
    r[1] = gmulLookupDefine ( a[1], 2 ) ^ gmulLookupDefine ( a[0], 1 ) ^ gmulLookupDefine ( a[3], 1 ) ^ gmulLookupDefine ( a[2], 3 );
    r[2] = gmulLookupDefine ( a[2], 2 ) ^ gmulLookupDefine ( a[1], 1 ) ^ gmulLookupDefine ( a[0], 1 ) ^ gmulLookupDefine ( a[3], 3 );
    r[3] = gmulLookupDefine ( a[3], 2 ) ^ gmulLookupDefine ( a[2], 1 ) ^ gmulLookupDefine ( a[1], 1 ) ^ gmulLookupDefine ( a[0], 3 );
}

void galois::invGmixColumn ( uint8_t *r )
{
    /*Perform invGmixColumn on a single column*/

    uint8_t a[4]; //Make holding location
    memcpy ( a, r, 4 ); //Copy original into holding

//Perform matrix multiplcation by {{14,9,13,11}{11,14,9,13}{13,11,14,9}{9,13,11,14}}
    r[0] = gmulLookupDefine ( a[0], 14 ) ^ gmulLookupDefine ( a[3], 9 ) ^ gmulLookupDefine ( a[2], 13 ) ^ gmulLookupDefine ( a[1], 11 );
    r[1] = gmulLookupDefine ( a[1], 14 ) ^ gmulLookupDefine ( a[0], 9 ) ^ gmulLookupDefine ( a[3], 13 ) ^ gmulLookupDefine ( a[2], 11 );
    r[2] = gmulLookupDefine ( a[2], 14 ) ^ gmulLookupDefine ( a[1], 9 ) ^ gmulLookupDefine ( a[0], 13 ) ^ gmulLookupDefine ( a[3], 11 );
    r[3] = gmulLookupDefine ( a[3], 14 ) ^ gmulLookupDefine ( a[2], 9 ) ^ gmulLookupDefine ( a[1], 13 ) ^ gmulLookupDefine ( a[0], 11 );
}

void galois::rotateStateR ( int offset )
{
    /*Method used by shiftRows to scramble rows*/

    uint8_t temp = state[offset+7]; //Backup last character

    for ( int c = 7;c > 0;c-- ) //Shift all once
    {
        state[offset + c] = state[offset + c - 1];
    }

    state[offset] = temp; //Restore first character (wrapped)
}

void galois::rotateStateL ( int offset )
{
    /*Method used by invShiftRows to unscramble rows*/

    uint8_t temp = state[offset]; //backup first character

    for ( int c = 0;c < 7;c++ ) //Shift all once
    {
        state[offset + c] = state[offset + c + 1];
    }

    state[offset+7] = temp; //Restore last Character (wrapped)
}

void galois::rotate ( uint8_t *in )
{
    /*4-byte Rotate method for use in Key Expansion*/
    uint8_t temp = in[0];

    for ( int c = 0;c < 3;c++ )
    {
        in[c] = in[c + 1];
    }

    in[3] = temp;
}

void galois::shiftRows()
{
    /*ShiftRows method for performing Rijndaels shiftrow for 256-bit blocks*/
    for ( int i = 1;i < 4;i++ ) //For all 3 rows to be shifted (0 is never shifted)
    {
        switch ( i )
        {

        case 3:
            rotateStateL ( i*8 ); //Shift Row 3 4 times passing offset into state

        case 2:
            rotateStateL ( i*8 );

            rotateStateL ( i*8 ); //Shift Row 2 3 times

        case 1:
            rotateStateL ( i*8 ); //Shift Row one once
        }
    }
}

void galois::invShiftRows()
{
    /*InvShiftRows method for performing Rijndaels unshiftrow for 256-bit blocks*/

    for ( int i = 1;i < 4;i++ ) //For all 3 rows to be shifted (0 is never shifted)
    {
        switch ( i )
        {

        case 3:
            rotateStateR ( i*8 ); //UnShift Row 3 4 times passing offset into state

        case 2:
            rotateStateR ( i*8 );

            rotateStateR ( i*8 ); //UnShift Row 2 3 times

        case 1:
            rotateStateR ( i*8 ); //UnShift Row 1 once
        }
    }
}

uint8_t galois::rcon ( uint8_t in )
{
    /*Method performs the rcon operation (2 exponentiated) for key expansion*/
    uint8_t c = 1;

    if ( in == 0 ) return 0; //Anything to the 0 is 0

    while ( in != 1 )
    {
        c = gmulLookupDefine ( c, 2 ); //Continuous multiply by 2
        in--;
    }

    return c;
}

void galois::schedule_core ( uint8_t *in, uint8_t i )
{
    /*Schedule core is used in key expansion*/

    rotate ( in ); //Rotate all 4 bytes

    for ( int a = 0; a < 4; a++ ) //Substitute all 4 bytes
    {
        in[a] = SBox[in[a]];
    }

    in[0] ^= rcon ( i ); //XOR with 2 exponentiated to some power
}

void galois::expand_key()
{
    /*Key Expansion Method*/
    for ( int i = 0;i < 32;i++ )
    {
        key[i] = userKey[i]; //Set first 32 bytes to be the user inputted key
    }

    uint8_t t[4]; //Create temp 4 bytes

    int c = 32; //Start after the user inputted key
    uint8_t i = 1; //rcon exponentiation values stats at 1

    while ( c < 480 ) //For all expanded eky bytes
    {
        for ( int a = 0; a < 4; a++ ) //Base the first 4 bytes on the previous key blocks first 4 bytes
        {
            t[a] = key[a + c - 4];
        }

        if ( c % 32 == 0 ) //If processing the end of a key block
        {
            schedule_core ( t, i ); //Call schedule core to modify the 4 bytes in t
            i++;
        }

        if ( c % 32 == 16 ) //If processing the middle of a key block
        {
            for ( int a = 0; a < 4; a++ ) //Do a Subsitute for all 4 bytes
            {
                t[a] = SBox[t[a]];
            }
        }

        for ( int a = 0; a < 4; a++ ) //For all 4 bytes
        {
            key[c] = key[c - 32] ^ t[a]; //Key keys current entry is the result of xoring the last blocks 4 bytes and t
            c++;
        }
    }
}

#endif
