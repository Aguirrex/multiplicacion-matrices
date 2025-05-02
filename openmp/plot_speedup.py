import pandas as pd
import matplotlib.pyplot as plt

# Leer archivo CSV
df = pd.read_csv("results.csv")

# Crear gráfico para cada tamaño de matriz
sizes = df['Size'].unique()
plt.figure(figsize=(10, 6))

for size in sizes:
    subset = df[df['Size'] == size]
    subset = subset.sort_values('Threads')
    base_time = subset[subset['Threads'] == 1]['Time'].values[0]
    speedup = base_time / subset['Time']
    plt.plot(subset['Threads'], speedup, marker='o', label=f'{size}x{size}')

plt.xlabel("Número de Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Número de Threads para Multiplicación de Matrices")
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.savefig("speedup_plot.png")
plt.show()
