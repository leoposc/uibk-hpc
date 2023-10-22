#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "common/common.h"

// -- simulation code ---

int main(int argc, char **argv) {
  // 'parsing' optional input parameter = problem size
  int N = 200;
  if (argc > 1) {
    N = atoi(argv[1]);
  }
  int T = N * 400;
  printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);

  clock_t t_start = clock();

  // ---------- setup ----------

  // create a buffer for storing temperature fields
  Matrix A = createMatrix(N, N);

  // set up initial conditions in A
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      A[IDX(i, j, N)] = 273; // temperature is 0° C everywhere (273 K)
    }
  }

  // and there is a heat source in one corner
  int source_x = N / 4;
  int source_y = N / 4;
  A[IDX(source_x, source_y, N)] = 273 + 60;

  printf("Initial:\n");
  printTemperature(A, N);
  printf("\n");

  // ---------- compute ----------

  // create a second buffer for the computation
  Matrix B = createMatrix(N, N);

  // for each time step ..
  for (int t = 0; t < T; t++) {

    // .. we propagate the temperature over y
    for (long long i = 0; i < N; i++) {

      // .. and over x
      for (long long j = 0; j < N; j++) {

        // center stays constant (the heat is still on)
        if (i == source_x && j == source_y) {
          B[IDX(i, j, N)] = A[IDX(i, j, N)];
          continue;
        }

        // get temperature at current position
        value_t tc = A[IDX(i, j, N)];

        // get temperatures of adjacent cells
        value_t tl = (j != 0) ? A[IDX(i, j-1, N)] : tc;
        value_t tr = (j != N - 1) ? A[IDX(i, j+1, N)] : tc;
        value_t tu = (i != 0) ? A[IDX(i-1, j, N)] : tc;
        value_t td = (i != N - 1) ? A[IDX(i+1, j, N)] : tc;

        // compute new temperature at current position
        B[IDX(i, j, N)] = tc + 0.2 * (tl + tr + tu + td + (-4*tc));
      }
    }

    // swap matrices (just pointers, not content)
    Matrix H = A;
    A = B;
    B = H;

    // show intermediate step
    if (!(t % 10000)) {
      printf("Step t=%d:\n", t);
      printTemperature(A, N);
      printf("\n");
    }
  }

  releaseMatrix(B);

  clock_t t_stop = clock();

  // ---------- check ----------

  printf("Final:\n");
  printTemperature(A, N);
  printf("\n");

  int success = 1;
  for (long long i = 0; i < N; i++) {
    for (long long j = 0; j < N; j++) {
      value_t temp = A[IDX(i, j, N)];
      if (273 <= temp && temp <= 273 + 60)
        continue;
      success = 0;
      break;
    }
  }

  printf("Verification: %s\n", (success) ? "OK" : "FAILED");
  printf("Time: %lfs\n", (t_stop - t_start) / (double) CLOCKS_PER_SEC);

  // ---------- cleanup ----------

  releaseMatrix(A);

  // done
  return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
