#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>


// Function to fill a matrix with random integer numbers
void fillMatrix(int n, int **matrix) {
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            matrix[i][j] = rand() % 10;  // Random numbers between 0 and 9
        }
    }
}


void readMatrixFromFile(int n, int **matrix, const char* fileName) {
    FILE *file = fopen(fileName, "r");
    if (!file) {
        fprintf(stderr, "Cannot open file %s\n", fileName);
        exit(1);
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error leyendo archivo.\n");
                exit(1);
            }
        }
    }
    fclose(file);
}



void writeMatrixToFile(int n, int **matrix, const char* fileName) {
    FILE *file = fopen(fileName, "w");
    if (!file) {
        fprintf(stderr, "Cannot open file %s for writing\n", fileName);
        exit(1);
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

// Function to multiply matrices
void* multiplyStandard(int n, int** matrix1, int** matrix2, int** resultMatrix)
{
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            // The result matrix is already initialized to 0 outside
            for (int k = 0; k < n; k++){
                resultMatrix[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    return NULL;
}

// Function to multiply matrices simulating a transpose operation on the second matrix for cache optimization
void* multiplyTranspose(int n, int** matrix1, int** matrix2, int** resultMatrix)
{
    
    #pragma omp parallel for
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            // The result matrix is already initialized to 0 outside
            for (int k = 0; k < n; k++){
                resultMatrix[i][j] += matrix1[i][k] * matrix2[j][k];
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int numThreads = 1;
    int n = 2000;
    int useFiles = 0;
    int useTranspose = 0; // Flag for transpose method
    char fileA[100], fileB[100];
    char fileResult[100];
    strcpy(fileResult, "result.out"); // Default result file if not provided

    // Parse arguments
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "--files") == 0 && (i+2 < argc)) {
            useFiles = 1;
            strcpy(fileA, argv[++i]);
            strcpy(fileB, argv[++i]);
        } else if(strcmp(argv[i], "--result") == 0 && (i+1 < argc)) {
            strcpy(fileResult, argv[++i]);
        } else if(strcmp(argv[i], "--transpose") == 0) {
            useTranspose = 1;
        } else if(strcmp(argv[i], "--threads") == 0 && (i+1 < argc)) {
            numThreads = atoi(argv[++i]);
        }
    }
    omp_set_num_threads(numThreads);
    printf("Running with %d threads\n", numThreads);


    printf("Matrix size: %d x %d\n", n, n);

    // Initialization of seed for random numbers
    srand(time(NULL));
    
    // Dynamic memory allocation for matrices
    int **matrix1 = malloc(n * sizeof(int*));
    int **matrix2 = malloc(n * sizeof(int*));
    int **resultMatrix = malloc(n * sizeof(int*));
    if(matrix1 == NULL || matrix2 == NULL || resultMatrix == NULL){
        printf("Error in memory allocation.\n");
        return 1;
    }
    for (int i = 0; i < n; i++){
        matrix1[i] = malloc(n * sizeof(int));
        matrix2[i] = malloc(n * sizeof(int));
        resultMatrix[i] = malloc(n * sizeof(int));
        if(matrix1[i] == NULL || matrix2[i] == NULL || resultMatrix[i] == NULL){
            printf("Error in memory allocation.\n");
            return 1;
        }
    }
    
    if(useFiles) {
        readMatrixFromFile(n, matrix1, fileA);
        readMatrixFromFile(n, matrix2, fileB);
    } else {
        fillMatrix(n, matrix1);
        fillMatrix(n, matrix2);
    }
    
    // Initialize result matrix to zeros
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            resultMatrix[i][j] = 0;
        }
    }


    // Decide which kernel function to use
    void* (*kernelFunc)(int, int**, int**, int**);
    if(useTranspose) {
        kernelFunc = multiplyTranspose;
    } else {
        kernelFunc = multiplyStandard;
    }

    struct timespec start,end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    (*kernelFunc)(n, matrix1, matrix2, resultMatrix);

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Print which method was used
    if(useTranspose) {
        printf("Using transpose multiplication method\n");
    } else {
        printf("Using standard multiplication method\n");
    }
    
    double computeTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // Show computation time of the kernel
    printf("Multiplication computation time: %.9f seconds\n", computeTime);
    
    // Save result matrix to file
    writeMatrixToFile(n, resultMatrix, fileResult);

    for (int i = 0; i < n; i++){
        free(matrix1[i]);
        free(matrix2[i]);
        free(resultMatrix[i]);
    }
    free(matrix1);
    free(matrix2);
    free(resultMatrix);
    
    return 0;
}
