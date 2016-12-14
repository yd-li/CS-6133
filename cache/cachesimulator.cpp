/*
Author: Zhiwen Guan (zg540)
        Yuanda Li (yl3638)
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
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
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

struct config {
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache */
class Set
{
public:
    int count;
    vector<unsigned int> v_tag;
    vector<bool> v_valid;

    Set(int associativity)
    {
        count = 0;
        for (int i = 0; i < associativity; i++) {
            v_tag.push_back(0);
            v_valid.push_back(false);
        }
    }
};

class Cache
{
public:
    Cache(int block_size, int associativity, int cache_size)
    {
        this->block_size = block_size;
        this->associativity = associativity;
        this->cache_size = cache_size;
        this->offset_size = (int) log2(this->block_size);

        if (associativity == 0) {
            this->set_size = 1;
            this->index_size = 0;
            this->tag_size = 32;
            this->cache.push_back(new Set(cache_size * 1024 / block_size));
        } else {
            this->set_size = (cache_size * 1024) / (block_size * associativity);
            this->index_size = (int) log2(this->set_size) + this->offset_size;
            this->tag_size = 32 - this->index_size;
            for (int i = 0; i < this->set_size; i++)
                this->cache.push_back(new Set(associativity));
        }
    }

    int read_hit(bitset<32> accessaddr) {
        return find_block(accessaddr) == -1 ? RM : RH;
    }

    int write_hit(bitset<32> accessaddr)
    {
        return find_block(accessaddr) == -1 ? WM : WH;
    }

    // allocate an data into cache
    void allocate(bitset<32> accessaddr)
    {
        unsigned int tag = mapping_tag(accessaddr);
        unsigned int set_num = mapping_set_num(accessaddr);

        Set *set = this->cache[set_num];
        // Wether this is an empty block or need to evict data
        if (!set->v_valid[set->count])
            set->v_valid[set->count] = true;
        // Update tag and count
        set->v_tag[set->count] = tag;
        if (this->associativity != 0)
            set->count = (set->count + 1) % this->associativity;
        else
            set->count = (set->count + 1) % (this->cache_size * 1024 / this->block_size);
    }
private:
    vector<Set*> cache;
    int block_size, associativity, cache_size;
    int set_size, tag_size, index_size, offset_size;

    unsigned int mapping_tag(bitset<32> accessaddr)
    {
        bitset<32> tag (accessaddr.to_string().substr(0, this->tag_size));
        return (unsigned int) tag.to_ulong();
    }

    unsigned int mapping_set_num(bitset<32> accessaddr)
    {
        bitset<32> set_num (accessaddr.to_string().substr(this->tag_size, this->index_size - this->offset_size));
        return (unsigned int) set_num.to_ulong();
    }

    int find_block(bitset<32> accessaddr)
    {
        unsigned int tag = mapping_tag(accessaddr);
        unsigned int set_num = mapping_set_num(accessaddr);

        for (int i = 0; i < this->associativity; i++)
            if (cache[set_num]->v_valid[i] && cache[set_num]->v_tag[i] == tag)
                return i;

        return -1;
    }
};

int main(int argc, char* argv[])
{
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
        cache_params >> dummyLine;
        cache_params >> cacheconfig.L1blocksize;
        cache_params >> cacheconfig.L1setsize;
        cache_params >> cacheconfig.L1size;
        cache_params >> dummyLine;
        cache_params >> cacheconfig.L2blocksize;
        cache_params >> cacheconfig.L2setsize;
        cache_params >> cacheconfig.L2size;
    }

    // Implement by you:
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    Cache *L1 = new Cache(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
    Cache *L2 = new Cache(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState = 0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState = 0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";

    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line;
    string accesstype;      // the Read/Write access type from the memory trace;
    string xaddr;           // the address from the memory trace store in hex;
    unsigned int addr;      // the address from the memory trace store in unsigned int;
    bitset<32> accessaddr;  // the address from the memory trace store in the bitset;

    if (traces.is_open() && tracesout.is_open()) {
        while (getline (traces,line)) {   // read mem access file and access Cache
            istringstream iss(line);
            if (!(iss >> accesstype >> xaddr)) { break; }
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);
            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R") == 0) {
                // Implement by you:
                // read access to the L1 Cache,
                // and then L2 (if required),
                // update the L1 and L2 access state variable;
                L1AcceState = L1->read_hit(accessaddr);
                L2AcceState = L1AcceState == RH ? NA : L2->read_hit(accessaddr);

                if (L2AcceState == RM) // If L2 miss, should allocate data to L2
                    L2->allocate(accessaddr);

                if (L1AcceState == RM) // If l1 miss, should allocate data to L1
                    L1->allocate(accessaddr);
            } else {
                // Implement by you:
                // write access to the L1 Cache,
                // and then L2 (if required),
                // update the L1 and L2 access state variable;
                L1AcceState = L1->write_hit(accessaddr);
                L2AcceState = L1AcceState == WH ? NA : L2->write_hit(accessaddr);
            }

            tracesout << L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
        }
        traces.close();
        tracesout.close();
    }
    else
        cout<< "Unable to open trace or traceout file ";

    return 0;
}
