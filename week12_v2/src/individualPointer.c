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

    MPI_File file;
    char *username = getenv("USER");
    char filename[36];
    sprintf(filename, "/scratch/%s/output.txt", username);
    MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);

    // Create a buffer for each rank
    char *buffer = (char *)malloc(array_size * sizeof(char));

    // Populate the buffer with data (char)rankID + 'A'
    for (unsigned long int i = 0; i < array_size; i++) {
        buffer[i] = (char)rank + 'A';
    }

    

    for (int i = 0; i < 10; i++) {
        MPI_File_set_view(file, rank * array_size * sizeof(char), MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);
        // MPI_File_sync(file);

        MPI_File_write_all(file, buffer, array_size, MPI_CHAR, MPI_STATUS_IGNORE);
        // MPI_File_sync(file);
        MPI_File_set_view(file, rank * array_size * sizeof(char), MPI_CHAR, MPI_CHAR, "native", MPI_INFO_NULL);

        MPI_File_read_all(file, buffer, array_size, MPI_CHAR, MPI_STATUS_IGNORE);
    }

    MPI_File_close(&file);
    free(buffer);

    // benchmarking
    double end = MPI_Wtime();
    if (rank == 0) {
        printf("individualPointer::%f::%d::%ld\n", end - start, size, array_size);
    }

    MPI_Finalize();
    return 0;
}
