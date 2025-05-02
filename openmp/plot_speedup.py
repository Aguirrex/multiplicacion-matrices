import pandas as pd
import matplotlib.pyplot as plt

# read the csv file
df = pd.read_csv("results.csv")

# create a plot for each size of matrix
sizes = df['Size'].unique()
plt.figure(figsize=(10, 6))

# available markers
markers = ['o', 's', 'D', '^', 'v', '<', '>']

for i, size in enumerate(sizes):
    subset = df[df['Size'] == size]
    subset = subset.sort_values('Threads')
    base_time = subset[subset['Threads'] == 1]['Time'].values[0]
    speedup = base_time / subset['Time']
    
    # choose a marker cyclically if there are more sizes than markers
    marker = markers[i % len(markers)]
    
    plt.plot(subset['Threads'], speedup, marker=marker, label=f'{size}x{size}')

plt.xlabel("Número de Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Número de Threads para Multiplicación de Matrices")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("speedup_plot.png")
plt.show()
