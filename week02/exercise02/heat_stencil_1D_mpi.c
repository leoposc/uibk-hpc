#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

typedef double value_t;

#define RESOLUTION 120

// -- vector utilities --

typedef value_t *Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int N);

void printNumbers(Vector m, int N);

// -- simulation code ---

int main(int argc, char **argv) {

  // initialize the MPI environment
  MPI_Init(&argc, &argv);

  // 'parsing' optional input parameter = problem size
  int N = 2000;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 500;

  // create a communbicator specifically for this application
  MPI_Comm comm;
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);

  // fetch the rank of the running instance
  int rank;
  MPI_Comm_rank(comm, &rank);

  // fetch the total number of ranks (tasks) there are
  int size;
  MPI_Comm_size(comm, &size);

  if (rank == 0) {
    printf("N=%d\n", N);
    printf("T=%d\n", T);
    printf("R=%d\n", size);
  }

  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);
  printf("%d:PROC=%s\n", rank, processor_name);

  clock_t t_start = clock();

  // ----- overall simulation parameters ------

  int base_heat = 273;
  int source_x = N / 4;
  int source_heat = base_heat + 60;

  // ---------- setup (same on each node) ----------

  int K = N / size;
  int index_offset = rank * K;
  short left_neighbor = rank - 1;
  short right_neighbor = rank + 1;

  Vector A = createVector(K + 2);

  // set up initial conditions in A
  for (int i = 0; i < K + 2; i++) {
    A[i] = base_heat;
  }

  // set the heat source, if present
  long source_offset = source_x - index_offset;
  if (source_offset >= 0 && source_offset < K) {
    A[source_offset + 1] = source_heat;
  }

  // create a second buffer for the computation
  Vector B = createVector(K + 2);

  // ------ actual computation ------

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // .. we propagate the temperature
    for (long long i = 1; i < K + 1; i++) {

      // center stays constant (the heat is still on)
      if (i == source_offset + 1) {
        B[i] = A[i];
        continue;
      }

      // get temperature at current position
      value_t tc = A[i];

      // get temperatures of adjacent cells
      value_t tl = A[i-1];
      value_t tr = A[i+1];

      // compute new temperature at current position
      B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
    }

    // set the left border for task 0 (has no left neightbor)
    if (rank == 0) {
      B[0] = B[1];
    }
    
    // set the right border for the last task (has no right neighbor)
    if (rank == size - 1) {
      B[K+1] = B[K];
    }

    // swap matrices (just pointers, not content)
    Vector H = A;
    A = B;
    B = H;

    // ----- exchange border tiles with neighboring ranks ------

    // send the left-most value to the left neighbor
    if (left_neighbor >= 0) {
      double left_value = A[1];
      MPI_Send(&left_value, 1, MPI_DOUBLE, left_neighbor, rank, comm);
    }

    // send the right-most value to the right neighbor
    if (right_neighbor < size) {
      double right_value = A[K];
      MPI_Send(&right_value, 1, MPI_DOUBLE, right_neighbor, rank, comm);
    }

    // wait for the right-most value of the left neighbor
    if (left_neighbor >= 0) {
      double left_value;
      MPI_Recv(&left_value, 1, MPI_DOUBLE, left_neighbor, left_neighbor, comm, MPI_STATUS_IGNORE);
      A[0] = left_value;
    }

    // wait for the left-most value of the right neighbor
    if (right_neighbor < size) {
      double right_value;
      MPI_Recv(&right_value, 1, MPI_DOUBLE, right_neighbor, right_neighbor, comm, MPI_STATUS_IGNORE);
      A[K+1] = right_value;
    }
  }

  // allocate memory on the root rank
  Vector final = NULL;
  if (rank == 0) {
    final = createVector(N);
  }

  // collect the final results from each rank
  MPI_Gather(&A[1], K, MPI_DOUBLE, final, K, MPI_DOUBLE, 0, comm);

  clock_t t_stop = clock();

  if (rank == 0) {
    printTemperature(final, N);
    printf("\n");
    // printNumbers(final, N);

    double hash = 0;
    int success = 1;
    for (long long i = 0; i < N; i++) {
      value_t temp = final[i];
      // printf("%lf\n", temp);
      hash += temp;
      if (273 <= temp && temp <= 273 + 60) {
        continue;
      }
      success = 0;
      break;
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
    printf("Hash: %lf\n", hash);
    printf("Time: %lfs\n", (t_stop - t_start) / (double) CLOCKS_PER_SEC);

    // memory clean up
    releaseVector(final);
  }

  // memory clean up
  releaseVector(A);
  releaseVector(B);

  // MPI clean up
  MPI_Comm_free(&comm);
  MPI_Finalize();

  return EXIT_SUCCESS;
}

Vector createVector(int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) {
  free(m);
}

void printTemperature(Vector m, int N) {
  const char *colors = " .-:=+*^X#%@";
  const int numColors = 12;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;

  // set the 'render' resolution
  int W = RESOLUTION;

  // step size in each dimension
  int sW = N / W;

  // room
  // left wall
  printf("X");
  // actual room
  for (int i = 0; i < W; i++) {
    // get max temperature in this tile
    value_t max_t = 0;
    for (int x = sW * i; x < sW * i + sW; x++) {
      max_t = (max_t < m[x]) ? m[x] : max_t;
    }
    value_t temp = max_t;

    // pick the 'color'
    int c = ((temp - min) / (max - min)) * numColors;
    c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

    // print the average temperature
    printf("%c", colors[c]);
  }
  // right wall
  printf("X");
}

void printNumbers(Vector m, int N) {
  for (int i = 0; i < N; i++) {
    printf("%06.2lf\n", m[i]);
  }
}