# HPC Matrix Multiplication (Threads Version)

This implementation demonstrates a **multithreaded** version of the matrix multiplication program. It uses **POSIX Threads (pthreads)** to accelerate computation by splitting rows among multiple worker threads.

> This document serves as an overview similar to the sequential version's `NOTES.md`, but highlights the **threads-related changes**.

---

## Compilation

Compile the code using the standard GNU C compiler and the pthread library:

```bash
gcc threads.c -o threads -lpthread
```

### Compiler Optimization

We can also enable optimizations:

```bash
gcc -O3 threads.c -o threads -lpthread
```

This applies advanced optimizations, including loop unrolling and vectorization, which can enhance performance.

---

## Execution

```bash
./threads [n] [--files matrixA.txt matrixB.txt] [--result outputFile] [--transpose] [--doublethreads]
```

- `n`: Dimension of the square matrices (defaults to 2000 if not provided).
- `--files matrixA.txt matrixB.txt`: Reads two matrices from files instead of generating random matrices. If you specify `--files`, you must also provide `n`.
- `--result outputFile`: Writes the result matrix to the file specified (default: `result.out`).
- `--transpose`: Uses a transpose-based multiplication to optimize cache usage for the second matrix.
- `--doublethreads`: Doubles the number of threads compared to the number of available CPU cores.

Example commands:
```bash
# Simple run with default dimension (2000×2000), random matrices
./threads

# Specify dimension, read from files, save result to custom file
./threads 1024 --files matrixA.txt matrixB.txt --result my_result.out

# Perform transpose multiplication, use double the CPU count
./threads 800 --transpose --doublethreads
```

---

## Changes from Sequential to Multithreaded

1. **Thread Creation and Management**  
   - Each thread is assigned a slice (chunk) of the matrix rows to compute.  
   - The program creates a number of threads equal to either the number of CPU cores or twice that number (if `--doublethreads` is specified).

2. **ThreadData Structure**  
   - Holds information each thread needs: start row, end row, pointers to matrices, etc.

3. **Work Distribution**  
   - The rows are distributed evenly among the available threads. Any leftover rows due to integer division remainders are assigned to the first few threads.

4. **Kernel Functions**  
   - Two kernel functions, just like the sequential version, but **each** function processes only a subset of rows:
     - `multiplyChunkStandard`  
       Implements the standard matrix multiplication approach.  
     - `multiplyChunkTranspose`  
       Implements the transpose approach to improve cache locality.

5. **Timing**  
   - The timing now measures the period from thread creation until all threads have finished their computation (`pthread_join`).

6. **Command-Line Parsing**  
   - An additional flag, `--doublethreads`, is recognized to scale the number of worker threads.

---

## Key Implementation Details

- **Memory Allocation**  
  - Dynamically allocates three `n×n` matrices using `malloc()`.
  - Checks for successful allocation and frees all memory at the end to avoid leaks.

- **Random Matrix Initialization**  
  - `fillMatrix()` populates an `n×n` matrix with integers in the range [0..9].

- **I/O**  
  - `readMatrixFromFile()` and `writeMatrixToFile()` read/write matrices to files.

- **Using pthreads**  
  - `pthread_create()` spawns a thread with a specified kernel function and thread-specific data.  
  - `pthread_join()` waits for each thread to finish.

- **Performance Considerations**  
  - Using more threads than available processors may not always yield better performance unless certain hardware or performance constraints dictate a beneficial oversubscription.  
  - Necessitates careful balancing of work to avoid idle threads or cache contention.

---

## Example Usage

### 1. Random Matrices (Default Dimensions)
```bash
./threads
# Output:
# Matrix size: 2000 x 2000
# Using 8 thread(s)
# Using standard multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```

### 2. Using the Transpose Method
```bash
./threads 1000 --transpose
# Output:
# Matrix size: 1000 x 1000
# Using 8 thread(s)
# Using transpose multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```

### 3. Reading from Files with Double Threads
```bash
./threads 1500 --files matrixA.txt matrixB.txt --doublethreads --result result_double.out
# Output:
# Matrix size: 1500 x 1500
# Using 16 thread(s)  (assuming 8 CPU cores)
# Using standard multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```
