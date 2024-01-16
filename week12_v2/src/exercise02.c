#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define ITERATIONS 10

int main(int argc, char **argv) {

    unsigned long int array_size = 1024 * 1024 * atoi(argv[1]);
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    double start = MPI_Wtime();

    char buffer[array_size * size];
    char recvBuffer[array_size];

    for (unsigned long int i = 0; i < array_size; i++) {
        recvBuffer[i] = 'A' + (char)rank;
    }
    recvBuffer[array_size - 1] = '\0';

    // printf("Rank %d: %s\n", rank, recvBuffer);

    MPI_File file;
    if (rank == 0) {
        char *username = getenv("USER");
        char filename[36];
        sprintf(filename, "/scratch/%s/output.txt", username);
        MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);
    }

    for (int i = 0; i < ITERATIONS; i++) {
        // do stuff with data

        // collect data
        MPI_Gather(recvBuffer, array_size, MPI_CHAR, buffer, array_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            // write to file
            MPI_File_write(file, buffer, array_size * size, MPI_CHAR, MPI_STATUS_IGNORE);
            
            // read from file (for any reason?)
            MPI_File_read(file, buffer, array_size * size, MPI_CHAR, MPI_STATUS_IGNORE);
        }
        MPI_Scatter(recvBuffer, array_size, MPI_CHAR, buffer, array_size, MPI_CHAR, 0, MPI_COMM_WORLD);

        // printf("Rank %d, Iteration %d: %s\n", rank, i, recvBuffer);
    }

    if (rank == 0) {
        double end = MPI_Wtime();
        printf("Hybrid::%f::%d::%ld\n", end - start, size, array_size);
        MPI_File_close(&file);
    }

    MPI_Finalize();
    return 0;
}