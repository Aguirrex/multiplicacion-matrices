
# HPC Matrix Multiplication (Processes Version)

This implementation demonstrates a **multiprocess** version of the matrix multiplication program. It uses **POSIX processes** (`fork`, `wait`) to accelerate computation by splitting rows among multiple processes.


---

## Compilation

Compile the code using the standard GNU C compiler:

```bash
gcc processes.c -o processes
```

### Compiler Optimization

We can also enable optimizations:

```bash
gcc -O3 processes.c -o processes
```

This applies advanced optimizations, including loop unrolling and vectorization, which can enhance performance.

---

## Execution

```bash
./processes [n] [--files matrixA.txt matrixB.txt] [--result outputFile] [--transpose] [--doublethreads]
```

- `n`: Dimension of the square matrices (defaults to 2000 if not provided).  
- `--files matrixA.txt matrixB.txt`: Reads two matrices from files instead of generating random matrices. If you specify `--files`, you must also provide `n`.  
- `--result outputFile`: Writes the result matrix to the file specified (default: `result.out`).  
- `--transpose`: Uses a transpose-based multiplication to optimize cache usage for the second matrix.  
- `--doublethreads`: Doubles the number of processes compared to the number of available CPU cores (though “threads” is used in the flag name, the logic applies to processes here).

Example commands:

```bash
# Simple run with default dimension (2000×2000), random matrices
./processes

# Specify dimension, read from files, save result to custom file
./processes 1024 --files matrixA.txt matrixB.txt --result my_result.out

# Perform transpose multiplication, use double the CPU count
./processes 800 --transpose --doublethreads
```

---

## Changes from Sequential to Multiprocess

1. **Process Creation and Management**  
   - Each process is assigned a slice (chunk) of the matrix rows to compute.  
   - The program forks a number of child processes equal to either the number of CPU cores or twice that number if `--doublethreads` is specified.

2. **ProcessData Structure**  
   - Holds information each subprocess needs: start row, end row, pointers to matrices, etc.

3. **Work Distribution**  
   - The rows are distributed evenly among the available processes. Any leftover rows due to integer division remainders are assigned to the first few processes.

4. **Kernel Functions**  
   - Two kernel functions, just like the sequential version, but **each** function processes only a subset of rows:
     - `multiplyChunkStandard`  
       Implements the standard matrix multiplication approach.  
     - `multiplyChunkTranspose`  
       Implements the transpose approach to improve cache locality.

5. **Timing**  
   - The timing measures the period from process creation until all processes have finished their computation (`wait` calls).

6. **Command-Line Parsing**  
   - Similar to the sequential version, but an additional flag `--doublethreads` is recognized to scale the number of processes.

---

## Key Implementation Details

- **Shared Memory Allocation**  
  - Uses shared memory (`mmap`) to allow the child processes to access the same matrices without manual inter-process communication.
  - Cleans up memory after all processes complete.

- **Random Matrix Initialization**  
  - `fillMatrix()` populates an `n×n` matrix with integers in the range [0..9].

- **I/O**  
  - `readMatrixFromFile()` and `writeMatrixToFile()` read/write matrices to files.

- **Using Processes**  
  - `fork()` spawns separate processes that run the selected kernel function.  
  - Each child process works on its assigned rows, then exits. The parent `wait`s for them all before proceeding.

- **Performance Considerations**  
  - Using more processes than available processors may not always yield better performance unless certain hardware or performance constraints justify oversubscribing.  
  - Proper load balancing and sufficient shared resources are essential to avoid bottlenecks.

---

## Example Usage

### 1. Random Matrices (Default Dimensions)
```bash
./processes
# Output:
# Matrix size: 2000 x 2000
# Using 8 process(es)
# Using standard multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```

### 2. Using the Transpose Method
```bash
./processes 1000 --transpose
# Output:
# Matrix size: 1000 x 1000
# Using 8 process(es)
# Using transpose multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```

### 3. Reading from Files with Double Processes
```bash
./processes 1500 --files matrixA.txt matrixB.txt --doublethreads --result result_double.out
# Output:
# Matrix size: 1500 x 1500
# Using 16 process(es) (assuming 8 CPU cores)
# Using standard multiplication method
# Multiplication computation time: X.XXXXXXXXX seconds
```
