#ifndef TESTS_CPP
#define TESTS_CPP

#include "tests.h"

tests::tests ( uint8_t *sboxz, uint8_t *invsboxz )
{
    /*Initialize the Class by setting the pointers*/
    sbox = sboxz;
    invsbox = invsboxz;
}

tests::~tests()
{} //Unused default destructor

void tests::printSbox()
{
    /*Function for Printing the S-Box in a 16x16 table*/

    for ( int i = 0;i < 256;i++ )
    {
        cout << hex << ( uint16_t ) sbox[i] << "\t";

        if ( ( i + 1 ) % 16 == 0 ) cout << endl;
    }

    cout << dec;
}

void tests::testEntropy ( bool verbose )
{
    /*Function for reporting entropy of X, Y, and X|Y*/
    char strXFileName[33];
    char strYFileName[33];

    do
    {
        cout << "***XFileName Name: <=32 Chars****" << endl;
        cin.getline ( strXFileName, 33 );
    }

    while ( cin.gcount() == 1 );

    cout << "X: " << strXFileName << endl;

    do
    {
        cout << "***YFileName Name: <=32 Chars****" << endl;
        cin.getline ( strYFileName, 33 );
    }

    while ( cin.gcount() == 1 );

    cout << "Y: " << strYFileName << endl;

    ifstream X ( strXFileName, ios::in | ios::binary );
    ifstream Y ( strYFileName, ios::in | ios::binary );

    struct stat inputXStats;
    struct stat inputYStats;

    stat ( strXFileName, &inputXStats );
    stat ( strYFileName, &inputYStats );

    int inputXByteCount = inputXStats.st_size;
    int inputYByteCount = inputYStats.st_size;

    if ( inputXByteCount != inputYByteCount )
    {
        cout << "Files must be the same size! " << inputXByteCount << " != " << inputYByteCount << endl;
        return;
    }

    uint8_t tempDataX;

    uint8_t tempDataY;

    double probX[256];
    double probY[256];

    double jointProb[256][256];

    for ( int i = 0;i < 256;i++ )
    {
        probX[i] = 0.0;
        probY[i] = 0.0;

        for ( int j = 0;j < 256;j++ )
        {
            jointProb[i][j] = 0.0;
        }
    }

    for ( int i = 0;i < inputXByteCount;i++ )
    {
        X.read ( ( char* ) &tempDataX, sizeof ( tempDataX ) );
        probX[tempDataX]++;

        Y.read ( ( char* ) &tempDataY, sizeof ( tempDataY ) );
        probY[tempDataY]++;

        jointProb[tempDataX][tempDataY]++;
    }

    X.close();

    Y.close();

    for ( int i = 0;i < 256;i++ )
    {
        probX[i] /= inputXByteCount;

        if ( verbose ) cout << dec << "Xp(" << i << ") = " << probX[i] << endl;

        probY[i] /= inputYByteCount;

        if ( verbose ) cout << dec << "Yp(" << i << ") = " << probY[i] << endl;

        for ( int j = 0;j < 256;j++ )
        {
            jointProb[i][j] /= inputXByteCount;
        }
    }

    double condEntropy = 0.0;

    for ( int i = 0;i < 256;i++ )
    {
        for ( int j = 0;j < 256;j++ )
        {
            if ( ( jointProb[i][j] != 0.0 ) && ( probY[i] != 0.0 ) )
            {
                condEntropy += jointProb[i][j] * log ( probY[i] / jointProb[i][j] );
            }
        }
    }

    condEntropy /= log ( 2.0 );

    if ( condEntropy <= 0.0 ) condEntropy = 0.0;

    cout << "Cond Entropy = " << condEntropy << endl;

    double entropyX = 0.0;

    double entropyY = 0.0;

    for ( int i = 0;i < 256;i++ )
    {
        if ( probX[i] != 0.0 ) entropyX -= probX[i] * log ( probX[i] );

        if ( probY[i] != 0.0 ) entropyY -= probY[i] * log ( probY[i] );
    }

    entropyX /= log ( 2.0 );

    entropyY /= log ( 2.0 );
    cout << "EntropyX = " << entropyX << endl;
    cout << "EntropyY = " << entropyY << endl;
}

void tests::testAll ( bool verbose )
{
    testMidPoints ( verbose );
    testDistanceToCenter ( verbose );
    testNonLinearity ( verbose );
    testAvalanche ( verbose );
    testBitChanges ( verbose );
    testInvertibility();
}

void tests::testDistanceToCenter ( bool verbose )
{
    double midPointTotalXT = 0;
    double midPointTotalYT = 0;
    double midPointCountT = 0;
    double midPointTotalXF = 0;
    double midPointTotalYF = 0;
    double midPointCountF = 0;

    for ( int i = 0;i < 16;i++ )
    {
        for ( int j = 0;j < 16;j++ )
        {
            if ( sbox[i*16+j] > 0x7f )
            {
                midPointCountT++;
                midPointTotalXT += j;
                midPointTotalYT += i;
            }

            else
            {
                midPointCountF++;
                midPointTotalXF += j;
                midPointTotalYF += i;
            }
        }
    }

    double distanceT = sqrt ( pow ( midPointTotalXT / midPointCountT - 7, 2 ) + pow ( midPointTotalYT / midPointCountT - 7, 2 ) );

    double distanceF = sqrt ( pow ( midPointTotalXF / midPointCountF - 7, 2 ) + pow ( midPointTotalXF / midPointCountF - 7, 2 ) );
    cout << "1's Distance from MidPoint = " << distanceT << " 0's Distance from MidPoint = " << distanceF << endl;
//cout << distanceT << ";" << distanceF << endl;
}

void tests::testMidPoints ( bool verbose )
{
    double midPointTotalXT = 0;
    double midPointTotalYT = 0;
    double midPointCountT = 0;
    double midPointTotalXF = 0;
    double midPointTotalYF = 0;
    double midPointCountF = 0;

    for ( int i = 0;i < 16;i++ )
    {
        for ( int j = 0;j < 16;j++ )
        {
            if ( sbox[i*16+j] > 0x7f )
            {
                midPointCountT++;
                midPointTotalXT += j;
                midPointTotalYT += i;
            }

            else
            {
                midPointCountF++;
                midPointTotalXF += j;
                midPointTotalYF += i;
            }
        }
    }

    cout << "MidPoint for 1s: (" << midPointTotalXT / midPointCountT << "," << midPointTotalYT / midPointCountT << ") ";

    cout << "MidPoint for 0s: (" << midPointTotalXF / midPointCountF << "," << midPointTotalYF / midPointCountF << ")" << endl;
//cout << midPointTotalXT/midPointCountT << ";" << midPointTotalYT/midPointCountT << ";" << midPointTotalXF/midPointCountF << ";" << midPointTotalYF/midPointCountF << endl;
}

void tests::testInvertibility()
{
    for ( int i = 0; i < 256; i++ )
    {
        if ( invsbox[sbox[i]] != i )
        {
            cout << "Wrong value at " << i << endl;
        }

        else
        {
            if ( i == 255 ) cout << "OK all lookups succeeded" << endl;
        }
    }
}

void tests::BitARFWT()
{
    /*Walsh-Hadarman Transform Code from: www.ciphersbyritter.com*/
    int el1 = 0;
    int el2 = 0;
    int stradwid = 1;
    int bitARLast = 255;
    int blocks = 255;

    while ( stradwid != 0 )
    {
        el1 = 0;
        blocks >>= 1;

        for ( int block = 0; block <= blocks; block++ )
        {
            el2 = el1 + stradwid;

            for ( int pair = 0; pair < stradwid; pair++ )
            {
                int a = bits[ el1 ];
                int b = bits[ el2 ];
                bits[ el1 ] = a + b;
                bits[ el2 ] = a - b;
                el1++;
                el2++;
            }

            el1 = el2;
        }

        stradwid = ( stradwid + stradwid ) & bitARLast;
    }
}

void tests::testNonLinearity ( bool verbose )
{
    int minNL = 0;

    for ( int i = 0;i < 8;i++ )
    {
        for ( int j = 0;j < 256;j++ )
        {
            bits[j] = ( sbox[j] & BIT ( i ) ) == 0 ? 0 : 1;
        }

        BitARFWT();

        for ( int i = 1;i < 256;i++ )
        {
            if ( abs ( bits[i] ) > minNL ) minNL = abs ( bits[i] );
        }
    }

    minNL = 128 - minNL;

    cout << "MinNL = " << dec << minNL << endl;
}

void tests::testDataHistogram ( bool verbose )
{
    char inFile[128];
    char outFile[128];

    do
    {
        cout << "Enter the data filename: " << endl;
        cin.getline ( inFile, 128 );
    }

    while ( cin.gcount() == 1 );

    do
    {
        cout << "Enter the results filename: " << endl;
        cin.getline ( outFile, 128 );
    }

    while ( cin.gcount() == 1 );

    uint64_t Histogram[256];

    for ( int i = 0;i < 256;i++ ) Histogram[i] = 0;

    int n;

    struct stat results;

    n = stat ( inFile, &results );

    ifstream data ( inFile, ios::in | ios::binary );

    assert ( data );

    ofstream resultsFile ( outFile, ios::out );

    int c = 0;

    for ( int i = 0;i < results.st_size;i++ )
    {
        data.read ( ( char* ) &c, 1 );
        Histogram[c]++;
    }

    data.close();

    for ( int i = 0;i < 256;i++ )
    {
        resultsFile << hex << i << "," << dec << Histogram[i] << endl;

        if ( verbose ) cout << dec << Histogram[i] << "\t";

        if ( verbose ) if ( ( i + 1 ) % 16 == 0 ) cout << endl;
    }

    resultsFile << "Total Bytes: " << results.st_size << endl;

    if ( verbose ) cout << "Total Bytes: " << results.st_size << endl;

    resultsFile.close();
}

void tests::testBitChanges ( bool verbose )
{
    unsigned int totalBitChanges = 0;
    int count = 0;

    for ( int i = 0;i < 256;i++ )
    {
        count = 0;

        for ( int j = 0;j < 8;j++ )
        {
            if ( ( i&BIT ( j ) ) != ( sbox[i]&BIT ( j ) ) ) count++;

        }

        totalBitChanges += count;

        if ( verbose ) cout << "BitChange count for entry " << dec << i << " = " << count << "/" << ( 8 ) << " = " << count / ( float ) 8 << endl;
    }

    cout << "AvgBitChanges = " << totalBitChanges << "/" << ( 256*8 ) << " = " << totalBitChanges / ( float ) ( 256*8 ) << endl;
}

void tests::testAvalanche ( bool verbose )
{
    /*Tests the avalanche effect by counting bit changes for single bit changes in the input for all 256 entries*/
    unsigned int totalAvalanche = 0;
    int count = 0;
    int modified = 0;

    for ( int i = 0;i < 256;i++ )
    {
        count = 0;

        for ( int j = 0;j < 8;j++ )
        {
            modified = i ^ BIT ( j );

            for ( int k = 0;k < 8;k++ )
            {
                if ( ( sbox[i]&BIT ( k ) ) != ( sbox[modified]&BIT ( k ) ) ) count++;
            }
        }

        if ( verbose ) cout << "Avalanche for " << i << " = " << count << " / " << ( 8*8 ) << " = " << count / ( float ) ( 8*8 ) << endl;

        totalAvalanche += count;
    }

    cout << "Overall Avalanche = " << totalAvalanche << " / " << ( 8*8*256 ) << " = " << totalAvalanche / ( float ) ( 8*8*256 ) << endl;
}

#endif
