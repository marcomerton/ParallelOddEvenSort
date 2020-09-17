/*
 * ---- odd-even-par-dyn.cpp
 *
 * Parallel version of the Odd-even Sort using pthread with
 * auto scheduling for the work division.
 * Uses atomic variables and active barriers for thread synchronization.
 * Takes 4 or 5 arguments:
 *      N         : number of array elements
 *      niter     : upper bound for the number of iterations (optional)
 *      seed      : seed for the problem generation
 *      nw        : number of workers
 *      chunksize : size of a single computation
 *
 * Compile with
 * g++ -g -O3 -std=c++17 -ftree-vectorize -pthread odd-even-par-dyn.cpp -o odd-even-par-dyn
 * 
 * Compile with -DPRINT to display the vector after every phase
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
#include "TaskManager.cpp"
#include "Timer.cpp"

using namespace std;
using namespace std::chrono;


int main(int argc, char const *argv[])
{
    if(argc < 5) {
        cout << "Usage: " << argv[0] << " N [niter] seed nw chunksize" << endl;
        cout << "    N     : number of array elements" << endl
             << "    niter : number of iterations (optional)" << endl
             << "    seed  : seed for the random number generator (-1 => reversed vector)" << endl
             << "    nw    : number of workers" << endl
             << "    chunksize : size of a single computation (0 => chunksize = N/nw)" << endl;
        return -1;
        return -1;
    }

    // Command line arguments
    int N     = atoi(argv[1]);
    int niter = (argc >= 6) ? atoi(argv[2]) : 0;
    int seed  = (argc >= 6) ? atoi(argv[3]) : atoi(argv[2]);
    int nw    = (argc >= 6) ? atoi(argv[4]) : atoi(argv[3]);
    int chunksize = (argc >= 6) ? atoi(argv[5]) : atoi(argv[4]);
    if(chunksize <= 0) chunksize = N/nw;


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

    // Shared data structute for auto-scheduling
    TaskManager tm(chunksize, N);
    tm.set_index(0);

    auto worker_fun = [&] (int t)
    {
#if STATS
        unsigned long even_time = 0, even_runs = 0,       // Even phase statistics
                      even_overhead = 0, even_swaps = 0;
        unsigned long odd_time = 0, odd_runs = 0,         // Odd phase statistics
                      odd_overhead = 0, odd_swaps = 0;
        unsigned long barrier1_t = 0, barrier2_t = 0, update_t = 0; // Overhead statistics

        unsigned long temp;
#endif

        int swapped_prv, nswaps, start, end;
        while(!terminate) {
            // Even phase
            nswaps = 0;
#if STATS
            {   Timer t_even(&temp);
#endif
                while(tm.get_task(&start, &end)) {
#if STATS
                    { Timer t_scan(&temp);
#endif
                    nswaps += sort_couples(A, start, end);
#if STATS
                    } even_time += temp;
                    even_runs++;
#endif
                }
                swapped_prv = nswaps;
#if STATS
            }   even_overhead += temp;
                even_swaps += nswaps;
#endif

            // Barrier, wait for the master thread to reset it
#if STATS
            {   Timer t_b1(&temp);
#endif
                odd_barrier.wait_reset();
#if STATS
            }   barrier1_t += temp;
#endif

            // Odd phase
            nswaps = 0;
#if STATS
            {   Timer t_odd(&temp);
#endif
                while(tm.get_task(&start, &end)) {
#if STATS
                    { Timer t_scan(&temp);
#endif
                    nswaps += sort_couples(A, start, end);
#if STATS
                    } odd_time += temp;
                    odd_runs++;
#endif
                }
                swapped_prv |= nswaps;
#if STATS
            }   odd_overhead += temp;
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
                 << "\tAvg even run        " << ((float)even_time)/even_runs/1000 << " usecs ("
                 << even_swaps/even_runs << " swaps)" << endl
                 << "\tAvg task retrieve   "
                 << ((float)even_overhead-even_time)/even_runs/1000 << " usecs" << endl
                 << "\tAvg even phase      " << ((float)even_time)/iter/1000 << " usecs" << endl
                 << "\tAvg even scheduling " << ((float)even_overhead-even_time)/iter/1000 << " usecs" << endl
                 << "\tAvg barrier 1       " << ((float)barrier1_t)/iter/1000 << " usecs" << endl
                 << "\tAvg odd run         " << ((float)odd_time)/odd_runs/1000 << " usecs ("
                 << odd_swaps/odd_runs << " swaps)" << endl
                 << "\tAvg task retrieve   "
                 << ((float)odd_overhead-odd_time)/odd_runs/1000 << " usecs" << endl
                 << "\tAvg odd phase       " << ((float)odd_time)/iter/1000 << " usecs" << endl
                 << "\tAvg odd scheduling  " << ((float)odd_overhead-odd_time)/iter/1000 << " usecs" << endl
                 << "\tAvg update          " << ((float)update_t)/iter/1000 << " usecs" << endl
                 << "\tAvg barrier 2       " << ((float)barrier2_t)/iter/1000 << " usecs" << endl << endl;
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

        // Wait for the end of even phase
        odd_barrier.wait_all_nomod();
#if PRINT
        cout << "EVEN  ";
        print_vector(A);
#endif

        // Reset task manager and barrier
        tm.set_index(1);
        odd_barrier.reset();

        // Wait for the end of odd phase
        even_barrier.wait_all_nomod();
#if PRINT
        cout << "ODD   ";
        print_vector(A);
#endif

        if(!swapped) break; // Check for termination
        // Reset task manager and barrier
        tm.set_index(0);
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