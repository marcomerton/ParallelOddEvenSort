/*
 * utilis.cpp
 * Contains some utility functions to be used among all implementations
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

using namespace std;

/*
 * Prints the content of the vector
 * 		v : vector to be printed
 */
void print_vector(vector<int> v) {
    for(auto it = v.begin(); it != v.end(); it++)
        cout << *it << " ";
    cout << endl;
}

/*
 * Fills the vector with random numbers
 *		v    : vector to be filled
 *		seed : seed for the random number generator
 */
void fill_random(vector<int> &v, int seed) {
	iota(v.begin(), v.end(), 0);
	shuffle(v.begin(), v.end(), default_random_engine(seed));
}

/*
 * Fills the vector such that the sorting can be performed in at
 * most 'niter' iterations
 *		v     : vector to be filled
 *		seed  : seed for the random number generator
 *		niter : upper bound for the number of iterations
 */
void fill_for_fixed_iterations(vector<int> &v, int seed, int niter) {
	iota(v.begin(), v.end(), 0);
	if(niter == 1) return;
	niter--;

	srand(seed);

	for(int i=0; i<(int)v.size(); i++) {
		if(i != v[i]) continue;

		int min_idx = max(0, i - 2*niter);
		int max_idx = min((int)v.size()-1, i + 2*niter);

		int idx = min_idx + rand() % (max_idx - min_idx);
		if(idx == v[idx]) swap(v[i], v[idx]);
	}
}

/*
 * Fills the vector in reversed order so that each couple of
 * elements needs to be swapped in every phase and in evert
 * iteration
 *		v : vector to be filled
 */
void fill_reversed(vector<int> &v) {
	iota(v.begin(), v.end(), 0);
	reverse(v.begin(), v.end());
}