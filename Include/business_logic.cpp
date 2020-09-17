#include <vector>

using namespace std;

/*
 * Classical sequential version
 * 'A' is the vector to be sorted
 * 'start' and 'end' are the extremes of the interval to work on
 */
inline int sort_couples(vector<int> &A, int start, int end) {
    int swapped = 0;
    for(int i=start; i<end-1; i+=2) {
        if(A[i] > A[i+1]) {
            swap(A[i], A[i+1]);
            swapped++;
        }
    }

    return swapped;
}


/*
 * Trying to exploit vectorization
 * Does not seem to work properly
 */
inline int sort_couples_vec(vector<int> &A, int start, int end) {
    int swapped = 0;

#pragma GCC ivdep
    for(int i=start; i<end; i+=2) {

        int first = A[i];
        int second = A[i+1];
        int toSwap = first > second;

        // Conditional assignments, can be vectorized
        A[i] = (toSwap) ? second : first;
        A[i+1] = (toSwap) ? first : second;

        // Reduce operation can be vectorized
        swapped += toSwap;
    }

    return swapped;
}