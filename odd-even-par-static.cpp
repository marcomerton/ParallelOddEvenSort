/*
 * ---- odd-even-par-static.cpp
 *
 * Parallel version of the Odd-even Sort using pthread with static
 * division of the work.
 * Uses atomic variables and active barriers for thread synchronization.
 * Takes 3 or 4 arguments:
 *      N     : number of array elements
 *      niter : upper bound for the number of iterations (optional)
 *      seed  : seed for the problem generation
 *      nw    : number of workers
 *
 * Compile with
 * g++ -g -O3 -std=c++17 -ftree-vectorize -pthread odd-even-par-static.cpp -o odd-even-par-static
 * 
 * Compile with -DPRINT to display the vector after every iteration
 * Compile with -DSTATS to print extended statistics (for each thread) at the end
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <atomic>
#include <cassert>
#include <mutex>
#include <thread>

#include "business_logic.cpp"
#include "utils.cpp"
#include "ActiveBarrier.cpp"
#include "Timer.cpp"

using namespace std;
using namespace std::chrono;


int main(int argc, char const *argv[])
{
    if(argc < 4) {
        cout << "Usage: " << argv[0] << " N [niter] seed nw" << endl;
        cout << "    N     : number of array elements" << endl
             << "    niter : number of iterations (optional)" << endl
             << "    seed  : seed for the random number generator (-1 => reversed vector)" << endl
             << "    nw    : number of workers" << endl;
        return -1;
    }

    // Command line arguments
    int N     = atoi(argv[1]);
    int niter = (argc >= 5) ? atoi(argv[2]) : 0;
    int seed  = (argc >= 5) ? atoi(argv[3]) : atoi(argv[2]);
    int nw    = (argc >= 5) ? atoi(argv[4]) : atoi(argv[3]);

    // Statistics
    unsigned long iter = 0;
#if STATS
    mutex print_m; // For mutual exclusive prints
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

    // Variable for the stopping condition
    atomic<int> swapped = 0;

    // Variables and structures for synchronization
    atomic<bool> terminate = false;
    ActiveBarrier even_barrier(nw), odd_barrier(nw);

    auto worker_fun = [&] (int t)
    {
#if STATS
        unsigned long even_time = 0, even_swaps = 0;  // Statistics for even phase
        unsigned long odd_time = 0, odd_swaps = 0;    // Statistics for odd phase
        unsigned long barrier1_t = 0, barrier2_t = 0, update_t = 0; // Overhead statistics

        unsigned long temp;
#endif

        int E = N/2; // Number of couples in the even phase
        int start_e = (t == 0) ? 0 : 2*( t*(E/nw) + min(E%nw, t) );
        int end_e   = (t == nw-1) ? 2*E : start_e + 2*( (E/nw) + (t < E%nw) );

        int O = (N-1)/2; // Number of couples in the odd phae
        int start_o = 1 + ((t == 0) ? 0 : 2*( t*(O/nw) + min(O%nw, t) ));
        int end_o   = (t == nw-1) ? 1 + 2*O : start_o + 2*( (O/nw) + (t < O%nw) );


        int swapped_prv, nswaps;
        while(!terminate) {
            // Even phase
#if STATS
            {   Timer t_even(&temp);
#endif
                nswaps = sort_couples(A, start_e, end_e);
                swapped_prv = nswaps;
#if STATS
            }   even_time += temp;
                even_swaps += nswaps;
#endif

            // Barrier, wait for all the workers to reach it
#if STATS
            {   Timer t_b1(&temp);
#endif
                odd_barrier.wait_all();
#if STATS
            }   barrier1_t += temp;
#endif

            // Odd phase
#if STATS
            {   Timer t_odd(&temp);
#endif
                nswaps = sort_couples(A, start_o, end_o);
                swapped_prv |= nswaps;
#if STATS
            }   odd_time += temp;
                odd_swaps += nswaps;
#endif

            // Atomicly update the shared variable
#if STATS
            {   Timer t_update(&temp);
#endif
                swapped |= swapped_prv;
#if STATS
            }   update_t += temp;
#endif

            // Barrier, wait for the master thread (main) to reset the barrier
#if STATS
            {   Timer t_b2(&temp);
#endif
                even_barrier.wait_reset();
#if STATS
            }   barrier2_t += temp;
#endif
        } // End of loop


#if STATS
        {
            unique_lock<mutex> print_lock(print_m);
            cout << "Worker " << t << ":" << endl
                 << "\tAvg even phase " << ((float)even_time)/iter/1000 << " usecs"
                 << " (" << even_swaps/iter << " swaps)" << endl
                 << "\tAvg barrier 1  " << ((float)barrier1_t)/iter/1000 << " usecs" << endl
                 << "\tAvg odd phase  " << ((float)odd_time)/iter/1000 << " usecs"
                 << " (" << odd_swaps/iter << " swaps)" << endl
                 << "\tAvg update     " << ((float)update_t)/iter/1000 << " usecs" << endl
                 << "\tAvg barrier 2  " << ((float)barrier2_t)/iter/1000 << " usecs" << endl << endl;
        }
#endif

        return;
    };

    // Start the workers
    vector<thread*> workers(nw);
    for(int i=0; i<nw; i++)
        workers[i] = new thread(worker_fun, i);

    while(true) {
        iter++;

        // Wait for the end of the iteration
        even_barrier.wait_all_nomod();
#if PRINT
        cout << "ITER  ";
        print_vector(A);
#endif

        // Check for termination
        if(!swapped) break;
        odd_barrier.reset();
        swapped = 0;
        even_barrier.reset();
    }

    terminate = true; // Send termination signal
    even_barrier.reset();
    for(int i=0; i<nw; i++)
        workers[i]->join();

    auto stop = high_resolution_clock::now();
    auto total_time = duration_cast<microseconds>(stop - start).count();


    cout << "Total time with " << nw << " workers: " << ((float)total_time)/1000.0 << " msecs" << endl;
    cout << "Iterations: " << iter << " (" << ((float)total_time)/iter << " usecs per iteration)" << endl;


    // Just to make sure it works for larger vectors
    assert(is_sorted(A.begin(), A.end()));

    return 0;
}