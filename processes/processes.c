#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

// Function to allocate a shared matrix of size n x n.
// It allocates an array of int* pointers (for rows) and one contiguous block for all elements.
int **allocate_shared_matrix(int n) {
    int **mat = malloc(n * sizeof(int *));
    if(mat == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    int *data = mmap(NULL, n * n * sizeof(int), PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if(data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        mat[i] = data + i * n;
    }
    return mat;
}

// Function to free a shared matrix allocated with allocate_shared_matrix
void free_shared_matrix(int **mat, int n) {
    if(munmap(mat[0], n * n * sizeof(int)) == -1) {
        perror("munmap");
    }
    free(mat);
}

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
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                fprintf(stderr, "Error reading file %s.\n", fileName);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(file);
}

void writeMatrixToFile(int n, int **matrix, const char* fileName) {
    FILE *file = fopen(fileName, "w");
    if (!file) {
        fprintf(stderr, "Cannot open file %s for writing\n", fileName);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++){
        for (int j = 0; j < n; j++){
            fprintf(file, "%d ", matrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

// Structure to hold process-specific data for multiplication.
typedef struct {
    int processID;
    int startRow;
    int endRow;
    int n;
    int **matrix1;
    int **matrix2;
    int **resultMatrix;
} ProcessData;

// Function to multiply matrices
void multiplyChunkStandard(ProcessData *data) {
    int n = data->n;
    for (int i = data->startRow; i < data->endRow; i++){
        for (int j = 0; j < n; j++){
            for (int k = 0; k < n; k++){
                data->resultMatrix[i][j] += data->matrix1[i][k] * data->matrix2[k][j];
            }
        }
    }
}

// Function to multiply matrices simulating a transpose operation on the second matrix for cache optimization
void multiplyChunkTranspose(ProcessData *data) {
    int n = data->n;
    for (int i = data->startRow; i < data->endRow; i++){
        for (int j = 0; j < n; j++){
            for (int k = 0; k < n; k++){
                data->resultMatrix[i][j] += data->matrix1[i][k] * data->matrix2[j][k];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 2000;
    int useFiles = 0;
    int useTranspose = 0;   // Flag for transpose method
    int useDoubleThreads = 0;   // Flag for doubling the number of processes
    char fileA[100], fileB[100];
    char fileResult[100];
    strcpy(fileResult, "result.out"); // Default result file if not provided

    // Parse arguments
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    for (int i = 1; i < argc; i++){
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

    // Determine number of processes based on available CPUs.
    int numCPUs = (int) sysconf(_SC_NPROCESSORS_ONLN);
    int numProcesses = useDoubleThreads ? (2 * numCPUs) : numCPUs;

    printf("Matrix size: %d x %d\n", n, n);
    printf("Using %d process(es)\n", numProcesses);

    // Initialize random seed
    srand(time(NULL));

    // Allocate shared memory for matrices
    int **matrix1 = allocate_shared_matrix(n);
    int **matrix2 = allocate_shared_matrix(n);
    int **resultMatrix = allocate_shared_matrix(n);

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

    // Choose the multiplication kernel before starting timing.
    void (*kernelFunc)(ProcessData *);
    char *methodName;
    if (useTranspose) {
        kernelFunc = multiplyChunkTranspose;
        methodName = "transpose multiplication method";
    } else {
        kernelFunc = multiplyChunkStandard;
        methodName = "standard multiplication method";
    }

    // Compute row-chunk sizes for processes
    int baseChunk = n / numProcesses;
    int remainder = n % numProcesses;

    // Timing start (only for the multiplication kernel)
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Fork processes for parallel matrix multiplication
    int currentRow = 0;
    for (int p = 0; p < numProcesses; p++) {
        int rowsForThisProcess = baseChunk + (p < remainder ? 1 : 0);

        ProcessData data;
        data.processID = p;
        data.startRow = currentRow;
        data.endRow = currentRow + rowsForThisProcess;
        data.n = n;
        data.matrix1 = matrix1;
        data.matrix2 = matrix2;
        data.resultMatrix = resultMatrix;

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // In child process: perform assigned multiplication chunk
            kernelFunc(&data);
            exit(EXIT_SUCCESS); // Child exits after finishing its work.
        }
        // Parent: update row index and continue forking other processes.
        currentRow += rowsForThisProcess;
    }

    // Parent waits for all child processes
    for (int p = 0; p < numProcesses; p++) {
        wait(NULL);
    }

    // Timing end
    clock_gettime(CLOCK_MONOTONIC, &end);
    double computeTime = (end.tv_sec - start.tv_sec) +
                         (end.tv_nsec - start.tv_nsec) / 1e9;

    // Print the multiplication method and computation time
    printf("Using %s\n", methodName);
    printf("Multiplication computation time: %.9f seconds\n", computeTime);

    // Save the result matrix to file
    writeMatrixToFile(n, resultMatrix, fileResult);

    // Clean up shared memory allocations
    free_shared_matrix(matrix1, n);
    free_shared_matrix(matrix2, n);
    free_shared_matrix(resultMatrix, n);

    return 0;
}
