#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char **argv) {
    
    unsigned long int array_size = 1024 * 1024 * atoi(argv[1]);
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // benchmarking
    double start = MPI_Wtime();

    // Create a buffer for each rank
    char *buffer = (char *)malloc(array_size * sizeof(char));

    // Populate the buffer with data (char)rankID + 'A'
    for (unsigned long int i = 0; i < array_size; i++) {
        buffer[i] = (char)rank + 'A';
    }

    // Create a file with a name based on the rank
    char filename[36];
    char *username = getenv("USER");
    sprintf(filename, "/scratch/%s/output_rank%d.txt", username, rank);
    
    FILE *file = fopen(filename, "w");

    // repeat the write-read process 10 times
    for (int iter = 0; iter < 10; iter++) {
        // Write data to file
        fwrite(buffer, sizeof(char), array_size, file);

        // Read data from file
        rewind(file);
        fread(buffer, sizeof(char), array_size, file);

        // Print the data for verification
        // printf("Rank %d, Iteration %d: %.*s\n", rank, iter, array_size, buffer);
    }

    fclose(file);
    free(buffer);

    // benchmarking
    double end = MPI_Wtime();
    if (rank == 0) {
        printf("multipleFiles::%f::%d::%ld\n", end - start, size, array_size);
    }

    MPI_Finalize();
    return 0;
}
