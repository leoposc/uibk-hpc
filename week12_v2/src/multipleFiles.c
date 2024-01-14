#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE 10

int main(int argc, char **argv) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // benchmarking
    double start = MPI_Wtime();

    // Create a buffer for each rank
    char *buffer = (char *)malloc(ARRAY_SIZE * sizeof(char));

    // Populate the buffer with data (char)rankID + 'A'
    for (int i = 0; i < ARRAY_SIZE; i++) {
        buffer[i] = (char)rank + 'A';
    }

    // Create a file with a name based on the rank
    char filename[36];
    sprintf(filename, "/scratch/cb761047/output_rank%d.txt", rank);
    
    FILE *file = fopen(filename, "w");

    // repeat the write-read process 10 times
    for (int iter = 0; iter < 10; iter++) {
        // Write data to file
        fwrite(buffer, sizeof(char), ARRAY_SIZE, file);

        // Read data from file
        rewind(file);
        fread(buffer, sizeof(char), ARRAY_SIZE, file);

        // Print the data for verification
        // printf("Rank %d, Iteration %d: %.*s\n", rank, iter, ARRAY_SIZE, buffer);
    }

    fclose(file);
    free(buffer);

    // benchmarking
    double end = MPI_Wtime();
    // if (rank == 0) {
    //     printf("Time taken: %f\n", end - start);
    // }

    MPI_Finalize();
    return 0;
}
