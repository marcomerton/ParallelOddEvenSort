#include <iostream>
#include <atomic>

using namespace std;

class ActiveBarrier {
private:
    atomic<int> count;
    int reset_value;
  
public:
    // Initialize the barrier
    ActiveBarrier(int v) {
        reset_value = v;
        count = v;
    }

    // Reset the barrier's value
    void reset() {
        count = reset_value;
    }

    // Wait until the barrier is resetted
    void wait_reset() {
        count--;
        while(count != reset_value) ;
    }

    // Classical barrier wait
    void wait_all() {
        count--;
        while(count > 0) ;
    }

    // Wait at the barrier without modifying its value
    void wait_all_nomod() {
        while(count > 0) ;
    }
};