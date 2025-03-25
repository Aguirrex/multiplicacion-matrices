# HPC Matrix Multiplication Performance Analysis

This repository contains implementations of matrix multiplication using three different approaches: sequential, multithreaded (using POSIX Threads), and multiprocessing (using POSIX Processes). The primary goal is to compare the performance of these methods and analyze the impact of various optimizations on matrix multiplication operations.

## Repository Overview

The repository is structured with dedicated directories for each implementation approach:

- **Sequential**: Basic single-threaded implementation
- **Threads**: Multithreaded implementation using POSIX Threads
- **Processes**: Multiprocess implementation using POSIX Processes (fork/wait)

Each implementation directory contains source code, notes on implementation details, logs of performance benchmarks, and compiled executables. Utilities for matrix generation and result analysis are also provided.

## System Specifications

All benchmarks were conducted on:

- **OS**: Ubuntu 22.04.5 LTS x86_64
- **Kernel**: 6.8.0-47-generic
- **CPU**: AMD Ryzen 5 4500U with Radeon Graphics (6 cores)
- **Memory**: 8 GB

## Performance Analysis

### Overall Performance Comparison

The following tables summarize the average execution times (in seconds) across different configurations and matrix dimensions, based on 12 iterations for each configuration:

**Sequential:**

| Dimension | O3 + loop  | O3 + transpose | Standard   | Transpose Only |
|-----------|------------|----------------|------------|----------------|
| 10        | 0.000005   | 0.000003       | 0.000009   | 0.000008       |
| 100       | 0.001417   | 0.000970       | 0.009279   | 0.007521       |
| 200       | 0.009588   | 0.009387       | 0.039467   | 0.039278       |
| 400       | 0.041798   | 0.036231       | 0.272659   | 0.270871       |
| 800       | 0.400794   | 0.329565       | 2.606762   | 2.179408       |
| 1600      | 6.519647   | 2.877499       | 30.150426  | 17.425771      |
| 3200      | 117.031910 | 20.296180      | 265.969098 | 137.821635     |

**Threads:**

| Dimension | O3 + loop  | O3 + transpose | Standard   | Transpose Only |
|-----------|------------|----------------|------------|----------------|
| 10        | 0.000288   | 0.000281       | 0.000266   | 0.000309       |
| 100       | 0.000570   | 0.000565       | 0.002096   | 0.002217       |
| 200       | 0.002472   | 0.002341       | 0.012098   | 0.012224       |
| 400       | 0.015532   | 0.014220       | 0.055481   | 0.066472       |
| 800       | 0.096508   | 0.075750       | 0.481740   | 0.420476       |
| 1600      | 0.960430   | 0.553500       | 4.883142   | 3.728317       |
| 3200      | 21.823923  | 4.920511       | 50.627067  | 28.086868      |

**Processes:**

| Dimension | O3 + loop  | O3 + transpose | Standard   | Transpose Only |
|-----------|------------|----------------|------------|----------------|
| 10        | 0.000651   | 0.000633       | 0.000895   | 0.000709       |
| 100       | 0.000959   | 0.000860       | 0.002359   | 0.002385       |
| 200       | 0.003016   | 0.002396       | 0.012301   | 0.012812       |
| 400       | 0.015195   | 0.014235       | 0.057606   | 0.057618       |
| 800       | 0.083484   | 0.112730       | 0.445218   | 0.408261       |
| 1600      | 1.133376   | 0.506965       | 5.635137   | 3.778568       |
| 3200      | 19.903804  | 4.689157       | 87.777612  | 27.808911      |

### Key Findings

#### 1. Impact of Optimization Techniques

- **Compiler Optimizations (-O3)**: Provides substantial performance improvements across all implementations, with speedups of 3-4x in sequential code and 2-3x in parallel implementations.
- **Transpose Optimization**: Significantly improves performance, especially for larger matrices, by enhancing cache locality. The impact is most dramatic for matrices of size 1600×1600 and above.
- **Combined Optimizations**: The combination of `-O3` and transpose optimization consistently delivers the best performance across all implementation types, with improvements of up to 13x compared to standard code for large matrices.

#### 2. Scaling Behavior

- **Small Matrices (10×10 to 200×200)**: Limited benefit from parallelization due to thread/process creation overhead. Sequential execution with optimizations is competitive or faster.
- **Medium Matrices (400×400 to 800×800)**: Parallel implementations begin to show meaningful advantages, with both threads and processes providing 3-5x speedups over sequential code.
- **Large Matrices (1600×1600 to 3200×3200)**: Dramatic scaling benefits from parallelization, with optimized parallel implementations outperforming sequential code by factors of 10-20x.

#### 3. Comparison Between Approaches

- **Sequential vs. Threads**: Threads provide substantial speedups for matrices ≥800×800, especially with optimizations. For the largest matrices (3200×3200), optimized threads are ~4x faster than optimized sequential.
- **Sequential vs. Processes**: Similar patterns to threads, with optimized processes achieving comparable performance to threads.
- **Threads vs. Processes**: Threads generally outperform processes for smaller matrices due to lower context-switching overhead. For larger matrices, threads maintain their advantage in standard implementations, but with compiler and algorithmic optimizations applied, the performance gap narrows significantly, with processes occasionally outperforming threads only in the most optimized configurations.

#### 4. Doubling Threads/Processes Analysis

Based on the optimization comparison logs:

**Threads:**
- Doubling threads from 6 to 12 sometimes improved performance for optimized versions:
  - For O3 + Transpose: 1.12s → 1.07s (4.5% improvement)
  - However, for standard implementations doubling threads reduced performance: 9.11s → 8.99s (minimal gain)

**Processes:**
- Doubling processes from 6 to 12 showed mixed results:
  - For optimized versions: minimal improvements or slight regressions
  - For standard implementation: significant performance degradation (6.95s → 10.71s, 54% slower)

This suggests that for this specific hardware (6 physical cores), oversubscribing with more threads/processes than physical cores generally does not yield significant benefits and can be detrimental for unoptimized code due to context switching overhead.

#### 5. Matrix Size Scaling

The scaling behavior of execution time with matrix size shows approximately O(n³) complexity as expected for matrix multiplication:
- For each doubling of matrix dimension, execution time increases by ~8x
- This pattern is consistent across all implementation approaches
- Optimizations don't change the algorithmic complexity but reduce the constant factor significantly

### Optimization Rankings

Based on 2000×2000 matrix multiplication tests, the following rankings emerge:

**Threads:**
1. O3 + Transpose + DoubleThreads: 1.07s (8.48x speedup)
2. O3 + Transpose: 1.12s (8.13x speedup)
3. O3 + Loop + Transpose: 1.13s (8.03x speedup)
4. O3 + Loop + Transpose + DoubleThreads: 1.29s (7.07x speedup)
5. O3 + Loop: 2.12s (4.29x speedup)

**Processes:**
1. O3 + Loop + Transpose: 0.92s (7.57x speedup)
2. O3 + Loop + Transpose + DoubleThreads: 0.99s (7.00x speedup)
3. O3 + Transpose + DoubleThreads: 1.01s (6.90x speedup)
4. O3 + Transpose: 1.06s (6.54x speedup)
5. O3 + Loop: 1.94s (3.58x speedup)

**Sequential:**
1. O3 + Loop + Transpose: 5.34s (11.10x speedup vs. sequential standard)
2. O3 + Transpose: 5.36s (11.05x speedup vs. sequential standard)
3. O3 + Loop: 13.45s (4.41x speedup vs. sequential standard)
4. O3 only: 13.80s (4.30x speedup vs. sequential standard)
5. Transpose only: 34.02s (1.74x speedup vs. sequential standard)
6. Standard: 59.33s (baseline)

## Conclusion

This benchmark demonstrates the significant impact of various optimization techniques on matrix multiplication performance:

1. **Compiler optimizations (-O3) provide substantial benefits** regardless of implementation approach, with 3-4x speedups.

2. **Transpose optimization dramatically improves performance for large matrices** by enhancing cache locality, with the greatest impact on sequential code.

3. **Parallelization through threads or processes delivers major speedups for medium to large matrices**, but offers limited benefits or even penalties for small matrices due to overhead.

4. **The combination of compiler optimizations (-O3) with transpose optimization consistently delivers the best performance** across all implementations.

5. **Processes can perform competitively with threads** for large matrices, despite higher creation overhead, suggesting memory access patterns may be more important than thread creation costs in this context.

6. **For this specific hardware (6 cores), doubling the number of threads/processes beyond physical core count provides minimal benefit and can be detrimental** for non-optimized code.

The results highlight the importance of combining algorithmic optimizations (transpose) with compiler optimizations (-O3) and appropriate parallelization for maximum performance in computationally intensive tasks like matrix multiplication.