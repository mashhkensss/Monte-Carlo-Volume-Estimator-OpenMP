# Monte-Carlo-Volume-Estimator-OpenMP
This project implements a **parallelized Monte Carlo algorithm** to estimate the volume of a 3D [piriform-shaped body](https://mathworld.wolfram.com/PiriformSurface.html). The program leverages **OpenMP** for multi-threading.

---

## 🔧 Tech Stack
- **Language:** C++20  
- **Compiler:** `clang++`  
- **Parallelization:** OpenMP  
- **Random Generator:** Xorshift (thread-safe implementation)
- **Build type:** Release

---

## ✨ Features
- Monte Carlo simulation to estimate 3D volume of a piriform body  
- Configurable **parallel execution** with OpenMP  
- Custom **Xorshift RNG** for efficient pseudo-random number generation  
- Support for:
  - Single-thread (`--no-omp`)
  - Multi-thread with default threads (`--omp-threads default`)
  - Multi-thread with custom threads (`--omp-threads N`)
- Input/output file support (`--input`, `--output`)
- Performance benchmarking with execution time measurement

---

## 📖 Overview
This project was developed as part of a study on **parallel computing performance and behavior**.  
The main goal was not only to implement a Monte Carlo simulation for estimating the volume of a 3D shape,  
but also to **analyze how execution time depends on the number of threads, scheduling strategy, and chunk size** when using OpenMP.

Through this project, I explored:
- The effect of **static vs dynamic scheduling** on performance
- The impact of **thread count** on efficiency and overhead
- The trade-offs between **precision and speed** of different pseudo-random number generators
- Practical use of **OpenMP constructs** for synchronization and workload distribution

The project combines **theoretical study** and **practical benchmarking** to demonstrate how  
parallelization strategies influence program execution time on modern processors.

---

## Program result example:

When running the program with Threads = 12, schedule(dynamic, 100000), the result is:
```
1.25656
Time (12 thread(s)): 79.5231 ms
```
Processor: Apple M3 Pro  

--- 

### Description of OpenMP constructs:

`OpenMP` is a set of compiler directives, libraries, and environment variables that allow writing parallel programs.  

In my program OpenMP is used for parallel generation of random points and checking whether they fall into the piriform figure.  

The following OpenMP constructs are used in the program:

* `omp_get_max_threads()`: function that returns the maximum number of threads that can be used in a parallel region if the number of threads is not explicitly specified.  

* `omp_get_thread_num()`: function that returns the number of the current thread in a parallel region.  
Thread numbers range from 0 to `omp_get_num_threads() - 1`.  

* `omp_get_wtime()`: function that returns the current time in seconds as a floating-point value.  

* `#pragma omp parallel num_threads(integer-expression)`: parallelizes the program, the code is executed by multiple threads.  
`num_threads(integer-expression)` sets the number of threads used in the parallel region.  

* `#pragma omp for schedule(kind, chunk_size)`: distributes loop iterations among threads.  
`schedule(kind, chunk_size)` means that each thread will receive dynamically or statically `(kind == dynamic/static)` distributed blocks of `<chunk_size>` iterations.  
The distribution works as follows: iterations are divided into blocks of size `chunk_size`, which are assigned to threads depending on the type of scheduling:  

  * **static**: blocks are statically assigned to threads in a round-robin fashion, i.e. each thread gets a set number of iterations at the start of the loop, and if iterations remain, the distribution continues. Planning is done once, and each thread “knows” the iterations it must perform. If `<chunk_size>` is not specified, iterations are divided into approximately equal blocks, each assigned to one thread.  

  * **dynamic**: blocks are distributed during execution. If a thread finishes processing its block of `<chunk_size>`, it is given the next one. If `<chunk_size>` is not specified, the default value is 1.  

* `#pragma omp atomic`: ensures atomic execution of an operation, preventing data races when updating the global hit counter, meaning only one thread can perform this operation at a given time.  


---

### Code description:
This program solves the problem of finding the volume of a piriform figure using the Monte Carlo method:  
it generates random points within a specified range obtained from the function `get_axis_range`, and checks whether they fall into the area defined by the `hit_test` function. Based on the number of points that fall into the piriform region, its volume is calculated.  

A pseudo-random number generator based on the **Xorshift** algorithm is used for random number generation. This generator is implemented in the `Xorshift` class.  

Justification for choosing this generator: in the C++ standard, I considered three thread-safe generators:  
`mt19937`, `minstd_rand`, and `ranlux`.  
- `mt19937` provides high precision and good randomness but requires significant computational resources and time.  
- `minstd_rand` is faster than `mt19937` but less precise, making piriform volume results less accurate.  
- `ranlux` provides the most precise results but is the slowest.  

Therefore, **Xorshift** was chosen as the most advantageous option: simple, fast, low resource consumption, and precise enough for this task. Moreover, it uses separate states for each thread, ensuring thread safety.  

The program supports parallel execution with OpenMP, which speeds up calculations by using multiple threads.  

The `main` function includes a command-line argument parser. 

Arguments supported:  
`--no-omp | --omp-threads[<num_threads> | default]` defines the execution mode:  
- `--no-omp` – single-thread execution without OpenMP  
- `--omp-threads default` – multi-thread version (default number of threads)  
- `--omp-threads 8` – multi-thread version (explicit number of threads)  

`--input [fname]` – input file name (contains number of points to generate).  

`--output [fname]` – output file name (where the volume result is saved).  

---

### Main function `calculate_volume()`:
1. Gets axis ranges from `get_axis_range()`  
2. Computes differences between max and min values for each axis  
3. Initializes timer with `double start_time = omp_get_wtime();`  
4. Depending on `use_omp` value:  
   - If `use_omp == false`: computation runs single-threaded  
   - If `use_omp == true`: parallel computation with OpenMP:  
     1. `#pragma omp parallel num_threads(num_threads)` creates a parallel region with given number of threads. Each thread initializes its RNG and local hit counter.  
     2. `#pragma omp for schedule(dynamic, 1000)` distributes loop iterations among threads.  
     3. `#pragma omp atomic` merges local hit counters into the global counter.  
5. Records `end_time = omp_get_wtime();`  
6. Computes volume from hit ratio and total points. Writes result to output file and prints runtime + volume to stdout.  

---

### Testing

#### Graph №1
Execution time with different thread counts, same schedule (no chunk size), N = 100000000:  

<img width="1266" height="768" alt="graph-1-omp" src="https://github.com/user-attachments/assets/5ab7a3e3-591e-4c96-8fa0-9ca424c077be" />

In most parts of the graph, dynamic and static scheduling show similar results.

The graph shows that when moving from 1 to 2 threads, the execution time increases. This may be due to the overhead of creating and managing threads, which turns out to be too significant compared to the gain from parallel execution. After that, the execution time starts to decrease as the number of threads increases. This happens thanks to parallel execution, which allows the workload to be distributed among the threads and reduces the overall runtime of the calculation. 

---

#### Graph №2
Execution time with threads = 12, varying chunk size (dynamic vs static), N = 100000000:  

<img width="911" height="667" alt="graph-2-omp" src="https://github.com/user-attachments/assets/e5ff522c-ac66-4fae-a869-0a91e1a6fff5" />

The blue line (dynamic): execution time with dynamic load balancing across threads.

The orange line (static): execution time with static load balancing across threads.

At the beginning, the execution time for dynamic scheduling is higher because with a small chunk size the overhead of distributing iterations is significant, which slows down performance. As the chunk size increases, the execution time for dynamic scheduling decreases until it reaches a block size of about 10³, after which it stabilizes at around 100 milliseconds. For static scheduling, execution time remains stable at around 100 milliseconds up to a chunk size of 10⁷. The graphs differ because dynamic scheduling initially has higher overhead from task management, which results in slower execution compared to static scheduling. At the end of the graph, when the chunk size reaches 10⁷, a sharp increase in execution time is observed (over 700 milliseconds). This happens because some threads start receiving very large blocks of iterations while others remain idle, meaning the threads are being used very inefficiently.

---

#### Graph №3
Comparison: no-omp vs omp threads=1, static/dynamic with varying chunk size, N = 100000000:  

<img width="992" height="603" alt="graph-3-omp" src="https://github.com/user-attachments/assets/d97f9d46-5746-493d-8def-c7fa3f57f001" />

For both dynamic scheduling (blue line) and static scheduling (orange line) without OpenMP, as well as for dynamic scheduling with a single thread (red line) and static scheduling with a single thread (green line), the values fluctuate within the range of 680–695 milliseconds. The results are almost identical in all cases, because if OpenMP is not used, the code runs on a single CPU core, which is equivalent to running OpenMP with Threads = 1. In both cases, the program behaves the same. The chunk size will not affect the no-omp run, since it is unrelated to the OpenMP implementation, and it also has no significant impact when running with Threads = 1, because schedule and chunk_size are used to distribute iterations among threads, and with only one thread there is nothing to distribute. 

---

#### Graph №4
Dynamic scheduling (`schedule(dynamic,1)`) with varying threads, N = 100000000:  

<img width="810" height="568" alt="graph-4-omp" src="https://github.com/user-attachments/assets/2ba4f72e-09cc-43f6-80f0-e20f4ff62228" />

On the graph, it can be seen that as the number of threads increases, the execution time of the task decreases significantly. This happens because parallel execution on multiple threads allows the workload to be divided among them and processed faster.

At first, the execution time decreases sharply when the number of threads increases (from 1 to 4), since each thread handles a part of the task, which greatly reduces the total runtime compared to single-thread execution.

After that, the improvement becomes less significant, especially beyond 8 threads, because further increasing the number of threads provides smaller benefits due to the overhead of thread management. 

---

