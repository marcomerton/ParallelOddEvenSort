#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Timer {
private:
    unsigned long *elapsed;
    high_resolution_clock::time_point start, stop;

public:
    Timer(unsigned long *p) : elapsed(p) {
        start = high_resolution_clock::now();
    }

    ~Timer() {
        stop = high_resolution_clock::now();

        if(elapsed)
            *elapsed = duration_cast<nanoseconds>(stop-start).count();
    }
};