#!/bin/bash

# Create logs directory if it doesn't exist
mkdir -p logs

# Create a log file for the output in the logs directory
LOG_FILE="logs/standard_vs_O3_floop.log"
> $LOG_FILE  # Clear the log file

# Function to append to log file
log() {
    echo "$1" | tee -a $LOG_FILE
}

# Log script execution start
log "=== Matrix Multiplication Comparison Script ==="
log "This is a test to prove if the -O3 -floop-interchange flag change the result of the matrix multiplication."
log "Started: $(date)"
log "------------------------------------------------"

# 1. Generate input matrices A.txt and B.txt (size 2000x2000)
log "Generating input matrices (2000x2000)..."
cd plain_matrices
python3 ../../utils/generate_matrix.py 2000 A.txt
python3 ../../utils/generate_matrix.py 2000 B.txt
cd ..

# Define result file paths in plain_matrices folder
NORMAL_RESULT="plain_matrices/result_without_O3_floop.out"
OPTIMIZED_RESULT="plain_matrices/result_with_O3_floop.out"

# 2. Compile sequential.c normally
log "Compiling standard version..."
gcc sequential.c -o executables/result_without_O3_floop

# 3. Run the normal version with matrices of size 2000, saving the result
log "Running standard version..."
./executables/result_without_O3_floop 2000 --files plain_matrices/A.txt plain_matrices/B.txt --result $NORMAL_RESULT 2>&1 | tee -a $LOG_FILE
# Extract the computation time
normal_time=$(grep "Multiplication computation time" $LOG_FILE | tail -1 | awk '{print $4}')

# 4. Compile with optimization flags
log "Compiling optimized version..."
gcc -O3 -floop-interchange sequential.c -o executables/sequential_with_O3_floop

# 5. Run the optimized version with the same matrices
log "Running optimized version..."
./executables/sequential_with_O3_floop 2000 --files plain_matrices/A.txt plain_matrices/B.txt --result $OPTIMIZED_RESULT 2>&1 | tee -a $LOG_FILE
# Extract the computation time
optimized_time=$(grep "Multiplication computation time" $LOG_FILE | tail -1 | awk '{print $4}')

# 6. Compare both outputs
log "Comparing results..."
diff -q $NORMAL_RESULT $OPTIMIZED_RESULT
if [ $? -eq 0 ]; then
    log "Both result matrices are the same."
else
    log "The result matrices differ."
fi

# 7. Calculate speedup
speedup=$(echo "scale=2; $normal_time / $optimized_time" | bc)
log "Speedup with optimization: ${speedup}x faster"
log "------------------------------------------------"
log "Results saved in: plain_matrices/"
log "Log file saved in: $LOG_FILE"
log "Completed: $(date)"