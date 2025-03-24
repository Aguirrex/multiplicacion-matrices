#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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

// Thread data structure
typedef struct {
    int threadID;
    int startRow;
    int endRow;
    int n;
    int **matrix1;
    int **matrix2;
    int **resultMatrix;
} ThreadData;

// Function to multiply matrices
void* multiplyChunkStandard(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int start = data->startRow;
    int end   = data->endRow;
    int n     = data->n;

    for (int i = start; i < end; i++){
        for (int j = 0; j < n; j++){
            // The result matrix is already initialized to 0 outside
            for (int k = 0; k < n; k++){
                data->resultMatrix[i][j] += data->matrix1[i][k] * data->matrix2[k][j];
            }
        }
    }
    return NULL;
}

// Function to multiply matrices simulating a transpose operation on the second matrix for cache optimization
void* multiplyChunkTranspose(void* arg) {
    ThreadData* data = (ThreadData*) arg;
    int start = data->startRow;
    int end   = data->endRow;
    int n     = data->n;

    for (int i = start; i < end; i++){
        for (int j = 0; j < n; j++){
            for (int k = 0; k < n; k++){
                data->resultMatrix[i][j] += data->matrix1[i][k] * data->matrix2[j][k];
            }
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int n = 2000;
    int useFiles = 0;
    int useTranspose = 0; // Flag for transpose method
    int useDoubleThreads = 0;   // Flag for doubling #threads
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
        } else if(strcmp(argv[i], "--doublethreads") == 0) {
            useDoubleThreads = 1;
        }
    }

    // Determine number of threads based on #processors
    int numCPUs = (int) sysconf(_SC_NPROCESSORS_ONLN);
    int numThreads = useDoubleThreads ? (2 * numCPUs) : numCPUs;

    printf("Matrix size: %d x %d\n", n, n);
    printf("Using %d thread(s)\n", numThreads);

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

    // Prepare for threading
    pthread_t *threads = malloc(numThreads * sizeof(pthread_t));
    ThreadData *threadData = malloc(numThreads * sizeof(ThreadData));

    // Decide which kernel function to use
    void* (*kernelFunc)(void*);
    if(useTranspose) {
        kernelFunc = multiplyChunkTranspose;
    } else {
        kernelFunc = multiplyChunkStandard;
    }

    // Compute row-chunk sizes
    int baseChunk = n / numThreads;
    int remainder = n % numThreads;
    
    // Start time measurement for kernel function
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Launch threads
    int currentRow = 0;
    for (int t = 0; t < numThreads; t++) {
        int rowsForThisThread = baseChunk + (t < remainder ? 1 : 0);
        threadData[t].threadID    = t;
        threadData[t].startRow    = currentRow;
        threadData[t].endRow      = currentRow + rowsForThisThread;
        threadData[t].n           = n;
        threadData[t].matrix1     = matrix1;
        threadData[t].matrix2     = matrix2;
        threadData[t].resultMatrix= resultMatrix;

        pthread_create(&threads[t], NULL, kernelFunc, (void*)&threadData[t]);
        currentRow += rowsForThisThread;
    }

    // Wait for all threads
    for (int t = 0; t < numThreads; t++) {
        pthread_join(threads[t], NULL);
    }

    // End timing
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

    // Free allocated memory
    free(threads);
    free(threadData);
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