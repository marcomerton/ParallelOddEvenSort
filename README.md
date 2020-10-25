# Parallel Odd-Even Sort

Project for the Parallel and Distributed Systems exam @Unipi.
The project consists of the implementation and testing of the Odd-Even Sort, both in a sequential and parallel fashion.

Four implementations are provided:
- ```odd-even-seq.cpp```: It is the sequential implementation, used to gather statistics and as a baseline for the evaluation of the parallel versions.
- ```odd-even-par-static.cpp```: It is the parallel implementation, using ```C++ pthreads```, with a static division of the workload. Each worker is assigned a continuous chunk of the input array to be sorted. Threads are synchronized at the end of each phase to make sure the boundary elements are updated before starting the next phase.
- ```odd-even-par-dyn.cpp```: It is the parallel implementation, using ```C++ pthreads```, with a dynamic schedulng policy. At each phase the array is divided in chunks of user defined size. Each thread retrieves one of such chunks from a shared data structure and applies a single sorting phase to the chunk, repeating the process until all the chunks have been processed.
- ```odd-even-ff.cpp```: It is the parallel implementaion using [FastFlow](https://github.com/fastflow/fastflow)
