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

  // print basic configuration
  if (rank == 0) {
    printf("N=%d\n", N);
    printf("T=%d\n", T);
    printf("R=%d\n", size);
  }

  // ----- overall simulation parameters ------

  int base_heat = 273;
  int source_x = N / 4;
  int source_heat = base_heat + 60;

  // ----- setup (same on each node) -----

  int K = N / size;
  long source_offset = source_x - K * rank;
  short has_source = source_offset >= 0 && source_offset < K;
  short left_neighbor = rank - 1;
  short right_neighbor = rank + 1;
  short has_left_neighbor = left_neighbor >= 0;
  short has_right_neighbor = right_neighbor < size;

  // create the array containing the current values
  Vector A = createVector(K + 2);

  // create a second buffer for the computation
  Vector B = createVector(K + 2);

  // allocate memory for the merged array on the root rank
  Vector merged = NULL;
  if (rank == 0) {
    merged = createVector(N);
  }

  // set up initial conditions in A
  for (int i = 0; i < K + 2; i++) {
    A[i] = base_heat;
  }

  // ------ actual computation ------
  clock_t t_start = clock();

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // apply the heat source, if present
    if (has_source) {
      A[source_offset + 1] = source_heat;
    }

    // --- start the non-blocking tile exchange in the background ---

    // non-blocking send left-most value to the left neighbor
    MPI_Request req_sleft;
    if (has_left_neighbor) {
      MPI_Isend(&A[1], 1, MPI_DOUBLE, left_neighbor, 0, comm, &req_sleft);
    }

    // non-blocking reviece of the left-most value of the right neighbor
    MPI_Request req_rright;
    if (has_right_neighbor) {
      MPI_Irecv(&A[K + 1], 1, MPI_DOUBLE, right_neighbor, 0, comm, &req_rright);
    } else {
      A[K + 1] = A[K];
    }

    // non-blocking send the right-most value to the right neighbor
    MPI_Request req_sright;
    if (has_right_neighbor) {
      MPI_Isend(&A[K], 1, MPI_DOUBLE, right_neighbor, 0, comm, &req_sright);
    }

    // non-blocking recieve of the right-most value of the left neighbor
    MPI_Request req_rleft;
    if (has_left_neighbor) {
      MPI_Irecv(&A[0], 1, MPI_DOUBLE, left_neighbor, 0, comm, &req_rleft);
    } else {
      A[0] = A[1];
    }

    // --- compute the tiles for which we have the data ---

    // .. we propagate the temperature for all tiles that do not use the border tiles
    for (long long i = 2; i < K; i++) {

      // if the array has a heat source at the current index, skip computation
      if (has_source && i == source_offset + 1) {
        continue;
      }

      // get temperature at current position
      value_t tc = A[i];

      // get temperatures of adjacent cells
      value_t tl = A[i - 1];
      value_t tr = A[i + 1];

      // compute new temperature at current position
      B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
    }

    // force the synchonization of the tile exchange
    if (has_left_neighbor) {
      MPI_Wait(&req_rleft, MPI_STATUS_IGNORE);
    }
    if (has_right_neighbor) {
      MPI_Wait(&req_rright, MPI_STATUS_IGNORE);
    }

    // recompute the border tiles using the recieved ghost cells
    B[1] = A[1] + 0.2 * (A[0] + A[2] + (-2 * A[1]));
    B[K] = A[K] + 0.2 * (A[K-1] + A[K+1] + (-2 * A[K]));

    // swap matrices (just pointers, not content)
    Vector H = A;
    A = B;
    B = H;

    // show intermediate step
    if (!(t % 10000)) {

      // merge all sub-arrays into a single one
      MPI_Gather(&A[1], K, MPI_DOUBLE, merged, K, MPI_DOUBLE, 0, comm);

      // print the merged array on the root node
      if (rank == 0) {
        printf("Step t=%d:\t", t);
        printTemperature(merged, N);
        printf("\n");
      }
    }

    // --- make sure the sends executed corretly ---
    if (has_left_neighbor) {
      MPI_Wait(&req_sleft, MPI_STATUS_IGNORE);
    }
    if (has_right_neighbor) {
      MPI_Wait(&req_sright, MPI_STATUS_IGNORE);
    }
  }

  // collect the final results from each rank
  MPI_Gather(&A[1], K, MPI_DOUBLE, merged, K, MPI_DOUBLE, 0, comm);

  clock_t t_stop = clock();

  if (rank == 0) {
    printf("Final:\t\t");
    printTemperature(merged, N);
    printf("\n");

    int success = 1;
    for (long long i = 0; i < N; i++) {
      value_t temp = merged[i];
      if (273 <= temp && temp <= 273 + 60) {
        continue;
      }
      success = 0;
      break;
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
    printf("Time: %lfs\n", (t_stop - t_start) / (double)CLOCKS_PER_SEC);

    // memory clean up
    releaseVector(merged);
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

void releaseVector(Vector m) { free(m); }

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
