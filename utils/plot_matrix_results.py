import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

def plot_results(csv_file):
    # Read the data
    df = pd.read_csv(csv_file)
    
    # Get unique dimensions and configurations
    dimensions = sorted(df['dimension'].unique())
    configurations = ['Standard', 'Transpose Only', 'O3 + loop', 'O3 + transpose']
    
    # Calculate average times for each dimension and configuration
    avg_results = {}
    for config in configurations:
        avg_results[config] = []
        for dim in dimensions:
            # Get average time for this dimension and configuration
            avg_time = df[(df['dimension'] == dim) & (df['configuration'] == config)]['time'].mean()
            avg_results[config].append(avg_time)
    
    # Set up the plot
    plt.figure(figsize=(12, 8))
    markers = ['o', 's', '^', 'D']
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']
    
    # Create summary table for the log
    print("\nAverage execution times (seconds):")
    print("=" * 80)
    print(f"{'Dimension':<12} {'Standard':<15} {'Transpose Only':<15} {'O3 + loop':<15} {'O3 + transpose':<15}")
    print("-" * 80)
    
    for i, dim in enumerate(dimensions):
        row = f"{dim:<12}"
        for config in configurations:
            row += f" {avg_results[config][i]:<15.6f}"
        print(row)
    
    # Plot each configuration
    for i, config in enumerate(configurations):
        plt.plot(dimensions, avg_results[config], marker=markers[i], color=colors[i], 
                 linewidth=2, markersize=8, label=config)
    
    # Add a logarithmic scale for better visualization
    plt.xscale('log', base=2)
    plt.yscale('log')
    
    # Add labels and title
    plt.xlabel('Matrix Dimension (N×N)', fontsize=14)
    plt.ylabel('Average Execution Time (seconds)', fontsize=14)
    plt.title('Matrix Multiplication Performance Comparison', fontsize=16)
    plt.grid(True, alpha=0.3)
    
    # Add a legend
    plt.legend(fontsize=12, loc='best')
    
    # Ensure all dimensions are shown on x-axis
    plt.xticks(dimensions, [str(d) for d in dimensions], rotation=45)
    
    # Save the plot
    plt.tight_layout()
    plt.savefig('plots/matrix_multiplication_performance.png', dpi=300)
    print("\nPlot saved as matrix_multiplication_performance.png")
    
    # Calculate and print speedups for the largest dimension
    largest_dim = max(dimensions)
    baseline_time = df[(df['dimension'] == largest_dim) & 
                     (df['configuration'] == 'Standard')]['time'].mean()
                     
    print(f"\nSpeedups for {largest_dim}x{largest_dim} matrices:")
    print("=" * 40)
    print(f"{'Configuration':<20} {'Speedup':<10}")
    print("-" * 40)
    
    for config in configurations:
        config_time = df[(df['dimension'] == largest_dim) & 
                       (df['configuration'] == config)]['time'].mean()
        speedup = baseline_time / config_time
        print(f"{config:<20} {speedup:<10.2f}x")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python plot_matrix_results.py results.csv")
        sys.exit(1)
    plot_results(sys.argv[1])