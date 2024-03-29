#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024 * 1024
#define ITERATIONS 10

int main(int argc, char **argv) {
    int Mbyte = 64;
    if(argc > 1) {
      Mbyte = atoi(argv[1]);
    }
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char buffer[BUFFER_SIZE * Mbyte * size];
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 'A' + (char)rank;
    }
    
    
    // create filename based on rank
    char filename[44];
    sprintf(filename, "/scratch/cb761012/io_benchmark/file_%d.txt", rank);
    

    MPI_File file;
    MPI_File_open(MPI_COMM_SELF, filename, MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);
		MPI_File_preallocate(file, BUFFER_SIZE*sizeof(char));

	  clock_t t_start = clock();
    for (int i = 0; i < ITERATIONS; i++) {
        MPI_File_write_at(file, 0, buffer, BUFFER_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_read_at(file, 0, buffer, BUFFER_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
    }
	  clock_t t_stop = clock();
    if(rank == 0) {
		  printf("$!%dMB{%d,%lfs}\n", Mbyte, size, (t_stop - t_start) / (double)CLOCKS_PER_SEC);
    }

    MPI_File_close(&file);
    MPI_Finalize();
    return 0;
}
