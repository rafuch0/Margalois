#include <iostream>
#include <math.h>
#include <fstream>
#include <cassert>
#include <sys/stat.h>

using namespace std;

int main(int argc, char *argv[])
{
    char inFile[128];

    uint64_t Histogram[256];
    for (int i=0;i<256;i++)Histogram[i]=0;

    int n;
    struct stat results;
    n = stat(argv[1], &results);
    ifstream data(argv[1], ios::in|ios::binary);
    assert(data);

    int c=0;

    for (int i=0;i<results.st_size;i++)
    {
        data.read((char*)&c,1);
        Histogram[c]++;
    }

    for (int i=0;i<256;i++)
    {
        cout << hex << i << "," << dec << Histogram[i] << "\n";
    }

    system("pause");
    return 0;
}
