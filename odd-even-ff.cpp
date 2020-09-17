/*
 * ---- odd-even-ff.cpp
 *
 * Odd-even Sort using FastFlow library
 * Takes 4 or 5 arguments:
 *      N         : number of array elements
 *      niter     : upper bound for the number of iterations (optional)
 *      seed      : seed for the problem generation
 *      nw        : number of workers
 *      chunksize : size of a single computation
 *
 * Compile with
 * g++ -g -O3 -std=c++17 -ftree-vectorize -pthread odd-even-ff.cpp -o odd-even-ff
 * 
 * Compile with -DPRINT to display the vector after every phase
 * Compile with -DSTATS to print extended statistics at the end
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>
#include <thread>
#include <atomic>

#include <ff/ff.hpp>
#include <ff/parallel_for.hpp>

#include "utils.cpp"
#include "Timer.cpp"

using namespace std;
using namespace std::chrono;
using namespace ff;


int main(int argc, char const *argv[])
{
    if(argc < 5) {
        cout << "Usage: " << argv[0] << " N [niter] seed nw chunksize" << endl;
        cout << "    N     : number of array elements" << endl
             << "    niter : number of iterations (optional)" << endl
             << "    seed  : seed for the random number generator (-1 => reversed vector)" << endl
             << "    nw    : number of workers" << endl
             << "    chunksize : size of a single computation" << endl
             << "                (=0 : static block scheduling)" << endl
             << "                (<0 : static cyclic scheduling)" << endl
             << "                (>0 : auto-scheduling)" << endl;
        return -1;
    }

    // Command line arguments
    int N     = atoi(argv[1]);
    int niter = (argc >= 6) ? atoi(argv[2]) : 0;
    int seed  = (argc >= 6) ? atoi(argv[3]) : atoi(argv[2]);
    int nw    = (argc >= 6) ? atoi(argv[4]) : atoi(argv[3]);
    int chunksize = (argc >= 6) ? atoi(argv[5]) : atoi(argv[4]);

    // Statistics
    unsigned long iter = 0;
#if STATS
    unsigned long even_time = 0, odd_time = 0;
    unsigned long temp;
#endif

    // Vector to be sorted
    vector<int> A(N);
    if(seed == -1) fill_reversed(A);
    else if(niter == 0) fill_random(A, seed);
    else fill_for_fixed_iterations(A, seed, niter);
#if PRINT
    cout << "INIT  ";
    print_vector(A);
#endif

    // Variable for the stopping condition
    int swapped = 1;

    auto body = [&] (const long i, int &s) {
        if(A[i] > A[i+1]) {
            swap(A[i], A[i+1]);
            s = 1;
        }
    };

    auto reduce = [&] (int &s, const int e) { s |= e; };

    ParallelForReduce<int> pfr(nw, true);
    pfr.disableScheduler();

    ffTime(START_TIME);

    int even_end = (N%2 == 0) ? N : N-1;
    int odd_end = (N%2 == 0) ? N-1 : N;

    while(swapped) {
        iter++;
        swapped = 0;

        // Even phase
#if STATS
        {   Timer t_even(&temp);
#endif
            pfr.parallel_reduce(
                    swapped, 0,
                    0, even_end, 2, chunksize,
                    body, reduce, nw);
#if STATS
        }   even_time += temp;
#endif
#if PRINT
        cout << "EVEN  ";
        print_vector(A);
#endif

        // Odd phase
#if STATS
        {   Timer t_odd(&temp);
#endif
            pfr.parallel_reduce(
                    swapped, 0,
                    1, odd_end, 2, chunksize,
                    body, reduce, nw);
#if STATS
        }   odd_time += temp;
#endif
#if PRINT
        cout << "ODD   ";
        print_vector(A);
#endif
    }

    ffTime(STOP_TIME);
    auto total_time = ffTime(GET_TIME);


    cout << "Total time with " << nw << " workers: " << ((float)total_time) << " msecs" << endl;
    cout << "Iterations: " << iter << " (" << ((float)total_time)/iter*1000 << " usecs per iteration)" << endl;
#if STATS
    cout << "Avg even phase  " << ((float)even_time)/iter/1000 << " usecs" << endl
         << "Avg odd phase   " << ((float)odd_time)/iter/1000 << " usecs" << endl;
#endif

    // Just to make sure it works for larger vectors
    assert(is_sorted(A.begin(), A.end()));

    return 0;
}