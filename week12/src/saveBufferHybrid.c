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
  char recvBuffer[BUFFER_SIZE];

  if (rank == 0) {
    for (int i = 0; i < BUFFER_SIZE*size; i++) {
      buffer[i] = 'A' + (i / BUFFER_SIZE);
    }
  }

  MPI_Scatter(buffer, BUFFER_SIZE, MPI_CHAR, recvBuffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);

  printf("Rank %d: %s\n", rank, recvBuffer);

  MPI_File file;
  if (rank == 0) {
    MPI_File_open(MPI_COMM_SELF, "hybrid.txt", MPI_MODE_CREATE | MPI_MODE_RDWR, MPI_INFO_NULL, &file);
  }

  for (int i = 0; i < ITERATIONS; i++) {
    MPI_Gather(recvBuffer, BUFFER_SIZE, MPI_CHAR, buffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank == 0) {
      MPI_File_write(file, buffer, BUFFER_SIZE * size, MPI_CHAR, MPI_STATUS_IGNORE);
    }
    MPI_Bcast(buffer, BUFFER_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
  }

  if (rank == 0) {
    MPI_File_close(&file);
  }

  MPI_Finalize();
  return 0;
}