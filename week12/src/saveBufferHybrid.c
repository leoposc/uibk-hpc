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
  char recvBuffer[BUFFER_SIZE * Mbyte];

  if (rank == 0) {
    for (int i = 0; i < BUFFER_SIZE*size; i++) {
      buffer[i] = 'A' + (i / BUFFER_SIZE);
    }
  }

  MPI_Scatter(buffer, BUFFER_SIZE, MPI_CHAR, recvBuffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);

  MPI_File file;
  if (rank == 0) {
    MPI_File_open(MPI_COMM_SELF, "/scratch/cb761012/io_benchmark/hybrid.txt", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);
  }

	clock_t t_start = clock();
  for (int i = 0; i < ITERATIONS; i++) {
    MPI_Gather(recvBuffer, BUFFER_SIZE, MPI_CHAR, buffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      MPI_File_write(file, buffer, BUFFER_SIZE * size, MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_Bcast(buffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
  }
  clock_t t_stop = clock();
  if(rank == 0) {
    printf("$!%dMB{%d,%lfs}\n", Mbyte, size, (t_stop - t_start) / (double)CLOCKS_PER_SEC);
  }


  if (rank == 0) {
    MPI_File_close(&file);
  }

  MPI_Finalize();
  return 0;
}