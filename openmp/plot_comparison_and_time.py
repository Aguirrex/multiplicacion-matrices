import pandas as pd
import matplotlib.pyplot as plt

# Load CSV files
machine1 = pd.read_csv('machine_1_results.csv')
machine2 = pd.read_csv('machine_2_results.csv')

# Normalize column names
machine1.columns = ['Threads', 'Size', 'Time']
machine2.columns = ['Threads', 'Size', 'Time']

# Define markers to cycle through
markers = ['o', 's', 'D', '^', 'v', '<', '>']

# Function to plot and save execution time vs matrix size
def plot_and_save(data, title, filename):
    plt.figure(figsize=(12, 6))  # más ancho para espacio extra
    thread_counts = sorted(data['Threads'].unique())
    for idx, thread_count in enumerate(thread_counts):
        subset = data[data['Threads'] == thread_count]
        marker = markers[idx % len(markers)]
        plt.plot(subset['Size'], subset['Time'], label=f'{thread_count} threads', marker=marker)

    plt.xlabel('Matrix Size (N x N)')
    plt.ylabel('Execution Time (seconds)')
    plt.title(title)
    plt.grid(True)

    # Mover la leyenda fuera del gráfico
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', borderaxespad=0.)
    plt.tight_layout(rect=[0, 0, 0.75, 1])  # dejar espacio a la derecha

    plt.savefig(filename, bbox_inches='tight')
    plt.close()


plot_and_save(machine1, 'Execution Time vs Matrix Size (Machine with 6 Threads)', 'machine1_execution_time.png')
plot_and_save(machine2, 'Execution Time vs Matrix Size (Machine with 32 Threads)', 'machine2_execution_time.png')

threads6_m1 = machine1[machine1['Threads'] == 6]
threads6_m2 = machine2[machine2['Threads'] == 6]

# Find common matrix sizes
common_sizes = set(threads6_m1['Size']).intersection(set(threads6_m2['Size']))
threads6_m1_common = threads6_m1[threads6_m1['Size'].isin(common_sizes)].sort_values('Size')
threads6_m2_common = threads6_m2[threads6_m2['Size'].isin(common_sizes)].sort_values('Size')

# Plot and save comparison
plt.figure(figsize=(10, 6))
plt.plot(threads6_m1_common['Size'], threads6_m1_common['Time'], label='Machine 1 (6 Threads)', marker='o')
plt.plot(threads6_m2_common['Size'], threads6_m2_common['Time'], label='Machine 2 (6 Threads)', marker='s')
plt.xlabel('Matrix Size (N x N)')
plt.ylabel('Execution Time (seconds)')
plt.title('Comparison: Machine 1 vs Machine 2 (6 Threads)')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('comparison_6_threads.png')
plt.close()
