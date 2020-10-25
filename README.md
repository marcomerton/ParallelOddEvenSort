# Parallel Odd-Even Sort

Project for the Parallel and Distributed Systems exam @Unipi.
The project consists of the implementation and testing of the [Odd-Even Sort](https://en.wikipedia.org/wiki/Odd%E2%80%93even_sort), both in a sequential and parallel fashion.

## Implementations

Four implementations are provided:
- ```odd-even-seq.cpp```: It is the sequential implementation, used to gather statistics and as a baseline for the evaluation of the parallel versions.
- ```odd-even-par-static.cpp```: It is the parallel implementation, using ```C++ pthreads```, with a static division of the workload. Each worker is assigned a continuous chunk of the input array to be sorted. Threads are synchronized at the end of each phase to make sure the boundary elements are updated before starting the next phase.
- ```odd-even-par-dyn.cpp```: It is the parallel implementation, using ```C++ pthreads```, with a dynamic schedulng policy. At each phase the array is divided in chunks of user defined size. Each thread retrieves one of such chunks from a shared data structure and applies a single sorting phase to the chunk, repeating the process until all the chunks have been processed.
- ```odd-even-ff.cpp```: It is the parallel implementaion using [FastFlow](https://github.com/fastflow/fastflow). It uses a [ParallelForReduce](https://github.com/fastflow/fastflow/blob/master/ff/parallel_for.hpp#L360) to implement a single phase. A single iteration of the algorithm includes two execution of the ```parallel_reduce``` method plus the check for the termination.

## Compiling Instructions
FastFlow library is required to compile the ```odd-even-ff.cpp``` code.
To install run
```
cd /usr/local
git clone https://github.com/fastflow/fastflow.git
ln -s ./fastflow/ff ./include
```

The ```makefile``` provided can compile all the versions.
> make odd-even-seq

Adding a ```-s``` after the file name will result in the code being compiled so that detailed statistics are printed after the execution, including the average time spent in each phase, the average number of swaps in each phase and the average time spent for the synchronization.
> make odd-even-seq-s

Adding a ```-p``` after the file name will result in the code being compiled so that the array content is printed after each phase, to be used for debug or explanatory purposes.
> make off-even-seq-p
