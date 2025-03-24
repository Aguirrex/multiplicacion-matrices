#!/bin/bash

# Force C locale for numeric operations to ensure decimal points are periods
export LC_NUMERIC=C

# Create a log file for detailed output
LOG_FILE="logs/threads_optimization_comparison.log"
> $LOG_FILE  # Clear the log file

# Create executables directory if it doesn't exist
EXEC_DIR="executables"
mkdir -p $EXEC_DIR
log() {
    echo "$1" >> $LOG_FILE
}
print() {
    echo "$1" | tee -a $LOG_FILE
}

print "=== Threads Matrix Multiplication Optimization Comparison ==="
print "Size: 2000x2000 | $(date +"%Y-%m-%d %H:%M:%S")"
print "------------------------------------------------"

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
    
    local binary_path="$EXEC_DIR/$binary_name"
    
    print "Testing: $description"
    
    # Compile (detailed output to log)
    log "Compiling: gcc $compiler_flags threads.c -o $binary_path -lpthread"
    gcc $compiler_flags threads.c -o $binary_path -lpthread &>> $LOG_FILE
    
    # Run (capture output to log)
    log "Running: $binary_path 2000 --result $result_file $run_args"
    $binary_path 2000 --result "$result_file" $run_args &>> $LOG_FILE
    
    # Extract execution time (latest line mentioning time)
    local exec_time=$(grep "Multiplication computation time" $LOG_FILE | tail -1 | awk '{print $4}')
    
    results+=("$result_file")
    times+=("$exec_time")
    descriptions+=("$description")
}

# 12 runs: No optimization, -O3, and -O3 -floop-interchange
# each combined with standard/transpose and single/double threads.
run_test "" "threads_normal"        ""                            "plain_matrices/result_standard.out"                   "Standard"
run_test "" "threads_transpose"     "--transpose"                "plain_matrices/result_transpose.out"                  "Transpose"
run_test "-O3" "threads_O3"         ""                            "plain_matrices/result_O3.out"                          "O3 only"
run_test "-O3" "threads_O3_trans"   "--transpose"                "plain_matrices/result_O3_transpose.out"               "O3 + Transpose"
run_test "-O3 -floop-interchange" "threads_O3_loop"        ""    "plain_matrices/result_O3_loop.out"                     "O3 + Loop"
run_test "-O3 -floop-interchange" "threads_O3_loop_trans" "--transpose" "plain_matrices/result_O3_loop_transpose.out"  "O3 + Loop + Transpose"

run_test "" "threads_normal_dbl"        "--doublethreads"                       "plain_matrices/result_standard_dbl.out"                   "Standard + DoubleThreads"
run_test "" "threads_transpose_dbl"     "--transpose --doublethreads"          "plain_matrices/result_transpose_dbl.out"                  "Transpose + DoubleThreads"
run_test "-O3" "threads_O3_dbl"         "--doublethreads"                       "plain_matrices/result_O3_dbl.out"                         "O3 + DoubleThreads"
run_test "-O3" "threads_O3_trans_dbl"   "--transpose --doublethreads"           "plain_matrices/result_O3_transpose_dbl.out"               "O3 + Transpose + DoubleThreads"
run_test "-O3 -floop-interchange" "threads_O3_loop_dbl"        "--doublethreads"             "plain_matrices/result_O3_loop_dbl.out"            "O3 + Loop + DoubleThreads"
run_test "-O3 -floop-interchange" "threads_O3_loop_trans_dbl"  "--transpose --doublethreads" "plain_matrices/result_O3_loop_transpose_dbl.out"  "O3 + Loop + Transpose + DoubleThreads"

# Display performance table
print "\n=== PERFORMANCE RESULTS ==="
print "+--------------------------------------+--------------+------------+"
print "| Implementation                        | Time (sec)   | Speedup    |"
print "+--------------------------------------+--------------+------------+"

# Store results in a temporary file
for ((i=0; i<${#times[@]}; i++)); do
    echo "${times[$i]} ${descriptions[$i]}" >> temp_times_threads.txt
done

# Use the first result (base run: standard) as baseline
baseline=${times[0]}

# Display sorted results
sort -n temp_times_threads.txt | while read -r time desc; do
    speedup=$(LC_NUMERIC=C echo "scale=2; $baseline / $time" | bc)
    printf "| %-36s | %12.2f | %10.2fx |\n" "$desc" "$time" "$speedup"
done | tee -a $LOG_FILE

print "+--------------------------------------+--------------+------------+"
print "Detailed logs available in: $LOG_FILE"
print "Executables saved in: $EXEC_DIR/"

# Clean up
rm temp_times_threads.txt