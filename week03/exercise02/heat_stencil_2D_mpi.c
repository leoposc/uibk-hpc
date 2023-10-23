#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

typedef double value_t;

#define RESOLUTION 120

#define IDX(i, j, N) ((i)*N + (j))

// -- matrix utilities --

typedef value_t* Matrix;

Matrix createMatrix(int N, int M);

void releaseMatrix(Matrix m);

void printTemperature(Matrix m, int N);

// -- simulation code ---

int main(int argc, char **argv) {

  // initialize the MPI environment
  MPI_Init(&argc, &argv);

  // 'parsing' optional input parameter = problem size
  int N = 200;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 100;

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

  // ---------- global setup ----------

  int absolute_source_x = N / 4;
  int absolute_source_y = N / 4;
  int base_heat = 273;
  int heat_source_heat = base_heat + 60;

  // ---------- local setup ----------

  // the local problem size
  int K = N / size;

  // neighbor configuration
  short above_neighbor = rank - 1;
  short below_neighbor = rank + 1;
  short has_above_neighbor = above_neighbor >= 0;
  short has_below_neighbor = below_neighbor < size;

  // heat source configuration
  int relative_source_x = absolute_source_x;
  int relative_source_y = absolute_source_y - K * rank;
  int has_source = relative_source_y >= 0 && relative_source_y < K;

  // create a local buffer for storing temperature fields
  Matrix A = createMatrix(K+2, N);

  // create a second local buffer for the computation
  Matrix B = createMatrix(K+2, N);

  // create the merged matrix that containls the aggregated result on the root node
  Matrix merged = NULL;
  if (rank == 0) {
    merged = createMatrix(N, N);
  }
  
  // set up initial conditions in A
  for (int i = 0; i < K+2; i++) {
    for (int j = 0; j < N; j++) {
      A[IDX(i, j, N)] = base_heat;
    }
  }

  // and there is a heat source in one corner
  if (has_source) {
    A[IDX(relative_source_y+1, relative_source_x, N)] = heat_source_heat;
  }
  
  // TODO use MPI_Wtime()
  clock_t t_start = clock();

  // ---------- compute ----------

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // apply the heat source, if present
    if (has_source) {
      A[IDX(relative_source_y+1, relative_source_x, N)] = heat_source_heat;
    }

    // --- non-blocking exchange the ghost cells ---

    // send top border cells to the above neighbor
    MPI_Request req_sa;
    if (has_above_neighbor) {
      MPI_Isend(&A[IDX(1, 0, N)], N, MPI_DOUBLE, above_neighbor, 0, comm, &req_sa);
    }

    // recieve top border cells from the below neighbor
    // store them in the bottom ghost cells
    // if there is no neighbor, replicate the border cells
    MPI_Request req_rb;
    if (has_below_neighbor) {
      MPI_Irecv(&A[IDX(K+1, 0, N)], N, MPI_DOUBLE, below_neighbor, 0, comm, &req_rb);
    } else {
      // TODO use array copy here?
      for (int x = 0; x < N; x++) {
        A[IDX(K+1, x, N)] = A[IDX(K, x, N)];
      }
    }

    // send bottom border cells to the below neighbor
    MPI_Request req_sb;
    if (has_below_neighbor) {
      MPI_Isend(&A[IDX(K, 0, N)], N, MPI_DOUBLE, below_neighbor, 0, comm, &req_sb);
    }

    // recieve bottom border cells from the above neighbor
    // store them in the above ghost cells
    // if there is no neighbor, replicate the border cells
    MPI_Request req_ra;
    if (has_above_neighbor) {
      MPI_Irecv(&A[IDX(0, 0, N)], N, MPI_DOUBLE, above_neighbor, 0, comm, &req_ra);
    } else {
      // TODO use array copy here?
      for (int x = 0; x < N; x++) {
        A[IDX(0, x, N)] = A[IDX(1, x, N)];
      }
    }

    // .. we propagate the temperature over y
    for (long long y = 2; y < K; y++) {

      // .. and over x
      for (long long x = 0; x < N; x++) {

        // if the current tile is a heat source, skip
        if (has_source && y == relative_source_y+1 && x == relative_source_x) {
          continue;
        }

        // get temperature at current position
        value_t tc = A[IDX(y, x, N)];

        // get temperatures of adjacent cells
        value_t tl = x-1 < 0 ? tc : A[IDX(y, x-1, N)];
        value_t tr = x+1 > N-1 ? tc : A[IDX(y, x+1, N)];
        value_t tu = A[IDX(y-1, x, N)];
        value_t td = A[IDX(y+1, x, N)];

        // compute new temperature at current position
        B[IDX(y, x, N)] = tc + 0.2 * (tl + tr + tu + td + (-4*tc));
      }
    }

    // force join the receive
    if (has_above_neighbor) {
      MPI_Wait(&req_ra, MPI_STATUS_IGNORE);
    }
    if (has_below_neighbor) {
      MPI_Wait(&req_rb, MPI_STATUS_IGNORE);
    }

    // comptue the values of the top border tiles
    for (int x = 0; x < N; x++) {
        int y = 1;

        // get temperature at current position
        value_t tc = A[IDX(y, x, N)];

        // get temperatures of adjacent cells
        value_t tl = x-1 < 0 ? tc : A[IDX(y, x-1, N)];
        value_t tr = x+1 > N-1 ? tc : A[IDX(y, x+1, N)];
        value_t tu = A[IDX(y-1, x, N)];
        value_t td = A[IDX(y+1, x, N)];

        // compute new temperature at current position
        B[IDX(y, x, N)] = tc + 0.2 * (tl + tr + tu + td + (-4*tc));
    }

    // comptue the values of the bottom border tiles
    for (int x = 0; x < N; x++) {
        int y = K;

        // get temperature at current position
        value_t tc = A[IDX(y, x, N)];

        // get temperatures of adjacent cells
        value_t tl = x-1 < 0 ? tc : A[IDX(y, x-1, N)];
        value_t tr = x+1 > N-1 ? tc : A[IDX(y, x+1, N)];
        value_t tu = A[IDX(y-1, x, N)];
        value_t td = A[IDX(y+1, x, N)];

        // compute new temperature at current position
        B[IDX(y, x, N)] = tc + 0.2 * (tl + tr + tu + td + (-4*tc));
    }

    // swap matrices (just pointers, not content)
    Matrix H = A;
    A = B;
    B = H;

    // show intermediate step
    if (!(t % 10000)) {
      MPI_Gather(&A[IDX(1, 0, N)], N * K, MPI_DOUBLE, merged, N * K, MPI_DOUBLE, 0, comm);
      if (rank == 0) {
        printf("Step t=%d:\n", t);
        printTemperature(merged, N);
        printf("\n"); 
      }
    }

    if (has_above_neighbor) {
      MPI_Wait(&req_sa, MPI_STATUS_IGNORE);
    }
    if (has_below_neighbor) {
      MPI_Wait(&req_sb, MPI_STATUS_IGNORE);
    }
  }

  clock_t t_stop = clock();

  // ---------- check ----------
  MPI_Gather(&A[IDX(1, 0, N)], N * K, MPI_DOUBLE, merged, N * K, MPI_DOUBLE, 0, comm);

  if (rank == 0) {
    printf("Final:\n");
    printTemperature(merged, N);
    printf("\n");

    int success = 1;
    for (long long i = 0; i < N; i++) {
      for (long long j = 0; j < N; j++) {
        value_t temp = merged[IDX(i, j, N)];
        if (273 <= temp && temp <= 273 + 60) {
          continue;
        }
        success = 0;
        break;
      }
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
    printf("Time: %lfs\n", (t_stop - t_start) / (double) CLOCKS_PER_SEC);

    releaseMatrix(merged);
  }

  // ---------- cleanup ----------
  releaseMatrix(A);
  releaseMatrix(B);

  MPI_Finalize();

  // done
  return EXIT_SUCCESS;
}

Matrix createMatrix(int N, int M) {
  return malloc(sizeof(value_t) * N * M);
}

void releaseMatrix(Matrix m) {
  free(m);
}

void printTemperature(Matrix m, int N) {
  const char *colors = " .-:=+*^X#%@";
  const int numColors = 12;

  // boundaries for temperature (for simplicity hard-coded)
  const value_t max = 273 + 30;
  const value_t min = 273 + 0;

  // set the 'render' resolution
  int W = RESOLUTION;

  // step size in each dimension
  int sW = N / W;

  // top border
  for (int i = 0; i < W + 2; i++) {
    printf("X");
  }
  printf("\n");

  // every line of the plane
  for (int i = 0; i < W; i++) {

    // left wall
    printf("X");

    // every tile in the line
    for (int j = 0; j < W; j++) {

      // get max temperature for this tile (in both x and y direction)
      value_t max_t = 0;
      for (int x = sW * i; x < sW * i + sW; x++) {
        for (int y = sW * j; y < sW * j + sW; y++) {
          max_t = (max_t < m[IDX(x, y, N)]) ? m[IDX(x, y, N)] : max_t;
        }
      }
      value_t temp = max_t;

      // pick the 'color'
      int c = ((temp - min) / (max - min)) * numColors;
      c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

      // print the average temperature
      printf("%c", colors[c]);
    }

    // right wall
    printf("X\n");
  }

  // bottom border
  for (int i = 0; i < W + 2; i++) {
    printf("X");
  }
  printf("\n");
}
