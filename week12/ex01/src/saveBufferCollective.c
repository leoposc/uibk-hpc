#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define ITERATIONS 10

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char buffer[BUFFER_SIZE * size];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 'A' + (char)rank;
    }
    
    printf("Rank %d: %s\n", rank, buffer);

    MPI_File file;
    MPI_File_open(MPI_COMM_WORLD, "collective.txt", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);

    for (int i = 0; i < ITERATIONS; i++) {
        MPI_File_write_ordered(file, buffer, BUFFER_SIZE*sizeof(char), MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_read_ordered(file, buffer, BUFFER_SIZE * sizeof(char), MPI_CHAR, MPI_STATUS_IGNORE);
    }
    printf("Rank %d: %s\n", rank, buffer);

    MPI_File_close(&file);
    MPI_Finalize();
    return 0;
}
