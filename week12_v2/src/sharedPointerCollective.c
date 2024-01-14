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

    MPI_File file;
    char *username = getenv("USER");
    char filename[36];
    sprintf(filename, "/scratch/%s/output.txt", username);
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);

    // Create a buffer for each rank
    char *buffer = (char *)malloc(ARRAY_SIZE * sizeof(char));

    // Populate the buffer with data (char)rankID + 'A'
    for (int i = 0; i < ARRAY_SIZE; i++) {
        buffer[i] = (char)rank + 'A';
    }

    // repeat the write-read process 10 times
    for (int iter = 0; iter < 10; iter++) {

        // Write data to file
        MPI_File_write_ordered(file, buffer, ARRAY_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);

        // Print the data for verification
        printf("Rank %d, Iteration %d: %s\n", rank, iter, buffer);

        // set the view to the beginning of the file for reading
        MPI_File_seek_shared(file, iter * ARRAY_SIZE * size, MPI_SEEK_SET);

        // Read data from file
        MPI_File_read_ordered(file, buffer, ARRAY_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
    }


    MPI_File_close(&file);
    free(buffer);

    // benchmarking
    double end = MPI_Wtime();
    if (rank == 0) {
        printf("Time taken: %f\n", end - start);
    }

    MPI_Finalize();
    return 0;
}
