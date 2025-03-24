#!/bin/bash

# Force C locale for numeric operations to ensure decimal points are periods
export LC_NUMERIC=C

# Create a log file for detailed output
LOG_FILE="logs/optimization_comparison.log"
> $LOG_FILE  # Clear the log file

# Create executables directory if it doesn't exist
EXEC_DIR="executables"
mkdir -p $EXEC_DIR
log "Created executables directory: $EXEC_DIR"

# Function to append only to log file (not stdout)
log() {
    echo "$1" >> $LOG_FILE
}

# Function to print important messages to both log and stdout
print() {
    echo "$1" | tee -a $LOG_FILE
}

print "=== Matrix Multiplication Optimization Comparison ==="
print "Size: 2000x2000 | $(date +"%Y-%m-%d %H:%M:%S")"
print "------------------------------------------------"

# # Generate matrices (only once)
# log "Generating matrices..."
# python3 ../../utils/generate_matrix.py 2000 A.txt 2>/dev/null
# python3 ../../utils/generate_matrix.py 2000 B.txt 2>/dev/null

# Arrays for storing results
declare -a results=()
declare -a times=()
declare -a descriptions=()

# Function to run test case with minimal output
run_test() {
    local compiler_flags="$1"
    local binary_name="$2"
    local run_args="$3"
    local result_file="$4"
    local description="$5"
    
    # Full path to the executable
    local binary_path="$EXEC_DIR/$binary_name"
    
    # Show test being executed
    print "Testing: $description"
    
    # Compile (detailed output to log only)
    log "Compiling: gcc $compiler_flags sequential.c -o $binary_path"
    gcc $compiler_flags sequential.c -o $binary_path &>> $LOG_FILE
    
    # Run (capture output to log)
    log "Running: $binary_path 2000 --result $result_file $run_args"
    $binary_path 2000 --result $result_file $run_args &>> $LOG_FILE
    
    # Extract execution time
    local exec_time=$(grep "Multiplication computation time" $LOG_FILE | tail -1 | awk '{print $4}')
    
    # Store results
    results+=("$result_file")
    times+=("$exec_time")
    descriptions+=("$description")
}

# Run all test cases
run_test "" "seq_normal" "" "plain_matrices/result_standardl.out" "Standard"
run_test "" "seq_transpose" "--transpose" "plain_matrices/result_transpose.out" "Transpose only"
run_test "-O3" "seq_O3" "" "plain_matrices/result_O3.out" "O3 only"
run_test "-O3" "seq_O3_transpose" "--transpose" "plain_matrices/result_O3_transpose.out" "O3 + Transpose"
run_test "-O3 -floop-interchange" "seq_O3_loop_transpose" "--transpose" "plain_matrices/result_O3_loop_transpose.out" "O3 + Loop + Transpose"
run_test "-O3 -floop-interchange" "seq_O3_loop" "" "plain_matrices/result_O3_loop.out" "O3 + Loop"

# Display performance table
print "\n=== PERFORMANCE RESULTS ==="
print "+-----------------------+--------------+------------+"
print "| Implementation        | Time (sec)   | Speedup    |"
print "+-----------------------+--------------+------------+"

# Sort results by execution time and create a temporary file
for ((i=0; i<${#times[@]}; i++)); do
    echo "${times[$i]} ${descriptions[$i]}" >> temp_times.txt
done

# Use the first result (standard) as the baseline for speedup calculations
baseline=${times[0]}

# Display sorted results using proper locale settings
sort -n temp_times.txt | while read -r time desc; do
    speedup=$(LC_NUMERIC=C echo "scale=2; $baseline / $time" | bc)
    printf "| %-21s | %12.2f | %10.2fx |\n" "$desc" "$time" "$speedup"
done | tee -a $LOG_FILE

print "+-----------------------+--------------+------------+"
print "Detailed logs available in: $LOG_FILE"
print "Executables saved in: $EXEC_DIR/"

# Clean up
rm temp_times.txt