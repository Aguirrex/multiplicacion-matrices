#!/bin/bash

# Configuration
DIMENSIONS=(10 100 200 400 800 1600 3200)
ITERATIONS=12
RESULTS_CSV="matrix_results.csv"
LOG_FILE="logs/matrix_performance.log"

# Create a log file and results CSV
echo "timestamp,dimension,iteration,configuration,time" > $RESULTS_CSV
echo "========== MATRIX MULTIPLICATION PERFORMANCE BENCHMARK ==========" > $LOG_FILE
echo "Started: $(date)" >> $LOG_FILE
echo "Configurations: Standard, Transpose Only, O3 + loop, O3 + transpose" >> $LOG_FILE
echo "Dimensions: ${DIMENSIONS[*]}" >> $LOG_FILE
echo "Iterations per configuration: $ITERATIONS" >> $LOG_FILE
echo "=================================================================" >> $LOG_FILE

# Function to log and echo
log() {
    echo "$1" | tee -a $LOG_FILE
}

# Function to extract execution time from output
extract_time() {
    grep "Multiplication computation time" | awk '{print $4}'
}

# Function to run a specific configuration
run_configuration() {
    local dimension=$1
    local iteration=$2
    local config=$3
    local compiler_flags=$4
    local runtime_args=$5
    local binary_name="executables/sequential_${config// /_}"
    
    # Compile
    gcc $compiler_flags -o $binary_name sequential.c
    
    # Run and capture output
    log "Running: Dimension=$dimension, Iteration=$iteration, Config=$config"
    output=$(./$binary_name $dimension $runtime_args 2>&1)
    
    # Extract time
    time=$(echo "$output" | extract_time)
    
    # Log the result
    echo "$output" >> $LOG_FILE
    echo "Time: $time seconds" >> $LOG_FILE
    echo "----------------------------------------" >> $LOG_FILE
    
    # Add to CSV
    echo "$(date +%s),$dimension,$iteration,\"$config\",$time" >> $RESULTS_CSV
}

log "Starting simulations..."

# Main loop
for dimension in "${DIMENSIONS[@]}"; do
    log "================================================================="
    log "Processing dimension: $dimension × $dimension"
    log "================================================================="
    
    for ((i=1; i<=ITERATIONS; i++)); do
        log "Iteration $i of $ITERATIONS for dimension $dimension"
        
        # Standard configuration
        run_configuration $dimension $i "Standard" "" ""
        
        # Transpose Only
        run_configuration $dimension $i "Transpose Only" "" "--transpose"
        
        # O3 + loop
        run_configuration $dimension $i "O3 + loop" "-O3 -floop-interchange" ""
        
        # O3 + transpose
        run_configuration $dimension $i "O3 + transpose" "-O3" "--transpose"
    done
done

log "All simulations completed."
log "Results saved to: $RESULTS_CSV"
log "Generating plot..."

# Run the Python script to generate the plot
python3 ../utils/plot_matrix_results.py $RESULTS_CSV

log "Benchmark completed. Plot saved as matrix_multiplication_performance.png"