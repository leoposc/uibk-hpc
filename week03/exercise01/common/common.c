#include <stdio.h>
#include <stdlib.h>

#include "common.h"

Matrix createMatrix(int N, int M) {
  return malloc(sizeof(value_t) * N * M);
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

void releaseMatrix(Matrix m) {
  free(m);
}
