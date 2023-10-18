#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#define RESOLUTION_WIDTH 50
#define RESOLUTION_HEIGHT 50

// -- vector utilities --

#define IND(y, x) ((y) * (N) + (x))

typedef double value_t;
typedef value_t *Vector;


Vector createVector(int N);
void releaseVector(Vector m);

double calc_nearby_heat_diff(double *m,int N, int x, int y);
void printTemperature(double *m, int N, int M);

// -- simulation code ---

int main(int argc, char **argv) {
    // 'parsing' optional input parameter = problem size
    int N = 200;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    int T = N * 10;
    printf("Computing heat-distribution for room size %dX%d for T=%d timesteps\n", N, N, T);

    // ---------- setup ----------

    // create a buffer for storing temperature fields
    Vector A = createVector(N);


    // set up initial conditions in A
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[IND(i,j)] = 273; // temperature is 0° C everywhere (273 K)
        }
    }

    // and there is a heat source
    int source_x = N / 4;
    int source_y = N / 4;
    A[IND(source_x,source_y)] = 273 + 60;

    printf("Initial:");
    //printTemperature(A, N, N);
    printf("\n");

    // ---------- compute ----------

    // create a second buffer for the computation
    Vector B = createVector(N);

    // for each time step ..
	  double startTime = MPI_Wtime();

    for (int t = 0; t < T; t++) {
        for (int y = 0; y < N; y++) {
            for (int x = 0; x < N; x++) {
                A[IND(y, x)] += calc_nearby_heat_diff(A, N, x, y);
            }
        }
        
        // keep heat source temp constant
        A[IND(source_x,source_y)] = 273 + 60;

        // every 100 steps show intermediate step
        if (!(t % 100)) {
            printf("Step t=%d\n", t);
            printTemperature(A, N, N);
            printf("\n");
        }
    }
	  double endTime = MPI_Wtime();


    // ---------- check ----------

    printf("Final:");
    //printTemperature(A, N, N);
    printf("\n");

    // simple verification if nowhere the heat is more then the heat source
    int success = 1;
    for (long long i = 0; i < N; i++) {
        for (long long j = 0; j < N; j++) {
            double temp = A[IND(i,j)];
            if (273 <= temp && temp <= 273 + 60)
                continue;
            printf("%f\n", temp);
            success = 0;
            break;
        }
    }

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");
	  printf("time: %2.4f seconds\n", endTime-startTime);

    // cleanup
    releaseVector(B);
    releaseVector(A);
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void printTemperature(double *m, int N, int M) {
    const char *colors = " .-:=+*^X#%@";
    const int numColors = 12;

    // boundaries for temperature (for simplicity hard-coded)
    const double max = 273 + 30;
    const double min = 273 + 0;

    // set the 'render' resolution
    int W = RESOLUTION_WIDTH;
    int H = RESOLUTION_HEIGHT;

    // step size in each dimension
    int sW = N / W;
    int sH = M / H;
    printf("step size: H: %d, W: %d\n", sH, sW);

    // upper wall
    printf("\t");
    for (int u = 0; u < W + 2; u++) {
        printf("X");
    }
    printf("\n");
    // room
    for (int i = 0; i < H; i++) {
        // left wall
        printf("\tX");
        // actual room
        for (int j = 0; j < W; j++) {
            // get max temperature in this tile
            double max_t = 0;
            for (int x = sH * i; x < sH * i + sH; x++) {
                for (int y = sW * j; y < sW * j + sW; y++) {
                    max_t = (max_t < m[IND(x,y)]) ? m[IND(x,y)] : max_t;
                }
            }
            double temp = max_t;

            // pick the 'color'
            int c = ((temp - min) / (max - min)) * numColors;
            c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

            // print the average temperature
            printf("%c", colors[c]);
        }
        // right wall
        printf("X\n");
    }
    // lower wall
    printf("\t");
    for (int l = 0; l < W + 2; l++) {
        printf("X");
    }
    printf("\n");
}

double calc_nearby_heat_diff(double *m,int N, int x, int y) {
    double current_temp = m[IND(y, x)];
    double temp_diff = 0;

    temp_diff += m[IND(x + 1, y)] - current_temp;
    temp_diff += m[IND(x, y + 1)] - current_temp;

    if (!(x == 0)) {
        temp_diff += m[IND(x - 1, y)] - current_temp;
    }
    if (!(y == 0)) {
        temp_diff += m[IND(x, y - 1)] - current_temp;
    }

    temp_diff /= 4;

    return temp_diff < 0 ? 0 : temp_diff;
}

Vector createVector(int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * N * N);
}

void releaseVector(Vector m) { free(m); }