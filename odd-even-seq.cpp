/*
 * ---- odd-even-seq.cpp
 *
 * Sequential version of the Odd-even Sort
 * Takes 2 or 3 arguments:
 *      N     : number of array elements
 *      niter : upper bound for the number of iterations (optional)
 *      seed  : seed for the problem generation
 *
 * Compile with
 * g++ -g -O3 -std=c++17 -ftree-vectorize odd-even-seq.cpp -o odd-even-seq
 * 
 * Compile with -DPRINT to display the vector after every phase
 * Compile with -DSTATS to print extended statistics at the end
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cassert>

#include "utils.cpp"
#include "business_logic.cpp"
#include "Timer.cpp"

using namespace std;
using namespace std::chrono;


int main(int argc, char const *argv[])
{
    if(argc < 3) {
        cout << "Usage: " << argv[0] << " N [niter] seed" << endl;
        cout << "    N     : number of array elements" << endl
             << "    niter : number of iterations (optional)" << endl
             << "    seed  : seed for the problem generation (-1 => reversed vector)" << endl;
        return -1;
    }

    // Command line arguments
    int N     = atoi(argv[1]);
    int niter = (argc >= 4) ? atoi(argv[2]) : 0;
    int seed  = (argc >= 4) ? atoi(argv[3]) : atoi(argv[2]);

    // Statistics
    unsigned long iter = 0;
#if STATS
    unsigned long even_time = 0, even_swaps = 0; // Statistics for even phase
    unsigned long odd_time = 0, odd_swaps = 0;   // Statistics for odd phase

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


    auto start = high_resolution_clock::now();

    // Ending index for the two phases
    int even_end = (N%2 == 0) ? N : N-1;
    int odd_end = (N%2 == 0) ? N-1 : N;

    int swapped = 1;
    int nswaps;
    while(swapped) {
        iter++;

        // Even phase
#if STATS
        {   Timer t_even(&temp);
#endif
            nswaps = sort_couples(A, 0, even_end);
#if STATS
        }   even_time += temp;
            even_swaps += nswaps;
#endif
            swapped = nswaps;
#if PRINT
        cout << "EVEN  ";
        print_vector(A);
#endif
        
        // Odd phase
#if STATS
        {   Timer t_odd(&temp);
#endif
            nswaps = sort_couples(A, 1, odd_end);
#if STATS
        }   odd_time += temp;
            odd_swaps += nswaps;
#endif
            swapped |= nswaps;
#if PRINT
        cout << "ODD   ";
        print_vector(A);
#endif
    }

    auto stop = high_resolution_clock::now();
    auto total_time = duration_cast<microseconds>(stop - start).count();


    cout << "Total time: " << ((float)total_time)/1000.0 << " msecs" << endl
         << "Iterations: " << iter << " (" << ((float)total_time)/iter << " usecs/iter)" << endl;
#if STATS
    cout << "Avg even phase " << ((float)even_time)/iter/1000 << " usecs"
         << " (" << ((float)even_time)/iter/(N/2) << " nsecs/function exec)"
         << " (" << even_swaps/iter << " swaps)" << endl
         << "Avg odd phase  " << ((float)odd_time)/iter/1000 << " usecs"
         << " (" << ((float)odd_time)/iter/(N/2) << " nsecs/function exec)"
         << " (" << odd_swaps/iter << " swaps)" << endl;
#endif


    // Just to make sure it works for larger vectors
    assert(is_sorted(A.begin(), A.end()));

    return 0;
}