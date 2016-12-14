#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;

int state_change[4][2] = {
/*          0: NT   1: T       */
/* 00 */{   0,      1},
/* 01 */{   0,      3},
/* 10 */{   0,      3},
/* 11 */{   2,      3}
};

int main(int argc, char* argv[])
{
    unsigned int m;
    ifstream params, traces;
    params.open(argv[1]);
    if (!params.eof())  // read config file
        params >> m;
    params.close();

    // Init variables
    unsigned int mask = ~(~0 << m); // Used to get the last m bits of an address by & operation
    vector<int> saturating_counter(pow(2, m), 3);

    ofstream tracesout;
    string outname = string(argv[2]) + ".out";
    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line, hex_addr;
    unsigned int actual_bit, addr;
    if (traces.is_open() && tracesout.is_open()) {
        while (getline(traces, line)) {
            istringstream iss(line);
            if (!(iss >> hex_addr >> actual_bit)) break;
            stringstream saddr(hex_addr);
            saddr >> std::hex >> addr;

            unsigned int index = addr & mask;
            int predicted = saturating_counter[index];
            // 1. Output predicted value to file
            tracesout << (predicted >= 2 ? "1" : "0") << endl;
            // 2. Update saturating counter
            saturating_counter[index] = state_change[predicted][actual_bit];
        }
        traces.close();
        tracesout.close();
    }
    else
        cout<< "Unable to open trace or traceout file";

    return 0;
}
