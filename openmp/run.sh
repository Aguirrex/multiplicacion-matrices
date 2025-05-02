#!/bin/bash

# Compilar el programa
gcc -fopenmp -O3 omp.c -o omp

# Crear archivo CSV con encabezados
echo "Threads,Size,Time" > results.csv

# Lista de tamaños de matrices
sizes=(20 200 400 800 1600 3200 6400)

# Para cada número de threads (de 1 a 6)
for threads in {1..6}; do
    echo "Testing with $threads thread(s)..."
    for size in "${sizes[@]}"; do
        echo "  Matrix size: $size x $size"
        output=$(./omp $size --threads $threads --transpose )
        # Extraer tiempo desde la salida
        time=$(echo "$output" | grep "Multiplication computation time" | awk '{print $4}')
        echo "$threads,$size,$time" >> results.csv
    done
done

echo "Benchmark completo. Resultados guardados en results.csv"
