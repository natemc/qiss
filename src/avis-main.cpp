#include <algorithm>
#include <bits.h>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <terminal_width.h>
#include <utility>
#include <vector>
#include <visible_buddy_allocator.h>
using namespace std;

#define L1(e) [&](auto&& x){ return e; }
#define L2(e) [&](auto&& x, auto&& y){ return e; }

namespace {
    struct Alloc { uint64_t i; void* p; uint64_t sz; };
    
    typedef VisibleBuddyAllocator::Block    Block;
    typedef VisibleBuddyAllocator::bucket_t bucket_t;

    const char dred   [] = "\033[30;48;5;1m";
    const char dgreen [] = "\033[30;48;5;2m";
    const char canvas [] = "\033[30;48;5;3m";
    const char dblue  [] = "\033[30;48;5;4m";
    const char purple [] = "\033[30;48;5;5m";
    const char aqua   [] = "\033[30;48;5;6m";
    const char gray   [] = "\033[30;48;5;7m";
    const char dgray  [] = "\033[30;48;5;8m";
    const char bred   [] = "\033[30;48;5;9m";
    const char bgreen [] = "\033[30;48;5;10m";
    const char byellow[] = "\033[30;48;5;11m";
    const char blue   [] = "\033[30;48;5;12m";
    const char pink   [] = "\033[30;48;5;13m";
    const char cyan   [] = "\033[30;48;5;14m";
    const char white  [] = "\033[30;48;5;15m";
    const char lgreen [] = "\033[30;48;5;193m";
    const char* colors[] = {
        dred, dgreen, canvas, dblue, purple, aqua, gray, dgray,
        bred, bgreen, byellow, blue, pink, cyan, white, lgreen,
    };
    const char reset  [] = "\033[0m";
}

int main(int argc, char* argv[]) {
    VisibleBuddyAllocator vba;
    vector<Alloc> allocs;
    uint64_t i = 0; // alloc index
    const int tw = terminal_width();
    const auto allocate = [&](uint64_t size) {
        const auto [p, sz] = vba.alloc(size);
        allocs.push_back({i++, p, sz});
        return pair{p, sz};
    };
    const auto deallocate = [&](void* p, uint64_t size=0) {
        const auto it = find_if(begin(allocs), end(allocs), L1(x.p == p));
        if (it == end(allocs)) {
            cerr << "No such alloc! " << p << '\n';
        } else {
            if (size && size != it->sz) {
                cerr << "Wrong size on free! " << p << ": tried to free " << size
                     << " when allocated was " << it->sz << '\n';
            } else {
                vba.free(it->p);
                allocs.erase(it);
            }
        }
    };
    const auto print = [&]() {
        if (empty(allocs)) return;
        sort(begin(allocs), end(allocs), L2(x.p < y.p));
        unordered_map<void*, uint64_t> regions;
        for (const Alloc& alloc: allocs) {
            Block* const block = vba.header(alloc.p);
            regions[block->region()] = vba.bucket_to_bytes(block->region_bucket());
        }
        if (size(regions) != 1) {
            cerr << "nyi: multiple regions\n";
        } else if (i > 10000) {
            cerr << "nyi: cannot render over 10K allocations\n";
        } else {
            const char* m = static_cast<char*>(begin(regions)->first); // memory offset
            const int iw = 4; // index width
            size_t c = 0; // color
            int x = 0; // column
            for (const Alloc& alloc: allocs) {
                const char* const p = static_cast<char*>(alloc.p) - 8;
                const char* const q = static_cast<char*>(alloc.p) + alloc.sz;
                for (; m < p; m += 8, ++x) cout << ' ';
                cout << colors[c] << setw(iw) << alloc.i;
                x += iw;
                for (m += iw * 8; m < q; m += 8, ++x) cout << ' ';
                cout << reset;
                c = (c + 1) % size(colors);
            }
            for (x = x % tw; x < tw; ++x) cout << ' ';
            cout << '\n';
        }
    };

    cout << "Allocator visualizer. Terminal width is " << tw << '\n';

    if (argc >= 2) {
        const bool interactive = argc > 2;
        const char* const file = argv[interactive? 2 : 1];
        ifstream in(file);
        if (!in) {
            cerr << "Could not read input file " << file << '\n';
        } else {
            int n = 0;
            string line;
            uint64_t base = 0, region = 0;
            while (getline(in, line)) {
                ++n;
                if (interactive) {
                    print();
                    cout << setw(4) << n << "| " << line;
                    string _;
                    if (!getline(cin, _)) break;
                }
                string action, addr;
                istringstream is(line);
                if (!(is >> action >> addr)) {
                    cerr << "Invalid line found in input file " << file << ": " << line << '\n';
                    break;
                }
                try {
                    const auto a = stoull(addr, nullptr, 16);
                    if (!base) base = a;
                    if (action == "FREE") {
                        string bucket_str;
                        if (!(is >> action >> bucket_str)) {
                            cerr << "Invalid bucket found in input file " << file << ": " << line << '\n';
                            break;
                        }
                        const int bucket(stoi(bucket_str));
                        assert(bucket < 48);
                        deallocate(bitcast<void*>(a - base + region + 8),
                                   vba.bucket_to_bytes(bucket_t(bucket)) - 8);
                    } else if (action == "LIST") {
                        uint64_t size;
                        if (is >> action >> size) {
                            const auto [p, sz] = allocate(size);
                            if (!region) region = bitcast<uint64_t>(p) - 8;
                            if (a - base + 8 != bitcast<uint64_t>(p) - region) {
                                cerr << "Addresses from file do not match behavior of allocator under test; ";
                                cerr << "SUT allocated at " << static_cast<void*>(static_cast<char*>(p) - region - 8) << '\n';
                                break;
                            }
                        } else {
                            cerr << "Invalid size found in input file " << file << ": " << line << '\n';
                            cerr << addr << '\t' << action << '\n';
                            break;
                        }
                    } else {
                        cerr << "Invalid command found in input file " << file << ": " << line << '\n';
                        break;
                    }
                } catch (const invalid_argument& e) {
                    cerr << "Invalid address found in input file: " << e.what() << '\n';
                    break;
                }
            }
            cout << "Processed " << n << " lines from file " << file << '\n';
        }
    }

    string line, cmd, arg;
    for (cout << "  " << flush; getline(cin, line) && line != "\\\\"; cout << "  " << flush) {
        istringstream is(line);
        if (!(is >> cmd))
            continue;
        if (cmd[0] == 'a') {
            if (!(is >> arg)) {
                cerr << "alloc command requires a positive integer arg\n";
            } else {
                try {
                    allocate(stoull(arg));
                } catch (const invalid_argument& e) {
                    cerr << "alloc command requires a positive integer arg: " << e.what() << '\n';
                }
            }
        } else if (cmd[0] == 'f') {
            if (!(is >> arg)) {
                cerr << "free command requires an alloc ID or hex addr arg\n";
            } else {
                try {
                    const auto a = stoull(arg, nullptr, 16);
                    if (a < size(allocs)) {
                        const auto it = find_if(begin(allocs), end(allocs), L1(x.i == a));
                        if (it == end(allocs)) {
                            cerr << "No such alloc: " << a << '\n';
                        } else {
                            vba.free(it->p);
                            allocs.erase(it);
                        }
                    } else {
                        deallocate(bitcast<void*>(a));
                    }
                } catch (const invalid_argument& e) {
                    cerr << "free command requires a hex addr arg: " << e.what() << '\n';
                }
            }
        } else if (cmd[0] == 'p') {
            print();
        } else {
            cerr << "Unknown command: " << cmd << '\n';
        }
    }
    for (const Alloc& alloc: allocs) vba.free(alloc.p);
}
