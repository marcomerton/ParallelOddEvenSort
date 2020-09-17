#include <iostream>
#include <atomic>

using namespace std;
using namespace std::chrono;

class TaskManager {
private:
    atomic<int> current_index;
    int chunksize, size;

public:
    TaskManager(int c, int s) : chunksize(c),size(s) {}

    void set_index(int v) {
        current_index = v;
    }

    bool get_task(int *s, int *e) {
        *s = current_index.fetch_add(chunksize);
        if(*s >= size) return false;
        *e = min(size, *s+chunksize);
        return true;
    }
};