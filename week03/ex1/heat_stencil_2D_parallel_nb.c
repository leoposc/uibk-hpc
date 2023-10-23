#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define RESOLUTION_WIDTH 50
#define RESOLUTION_HEIGHT 50

#define IND(y, x) ((y) * (N) + (x))

typedef double value_t;
typedef value_t *Vector;

Vector createVector(int N, int M);
void releaseVector(Vector m);
void initializeTemperature(Vector A, int M, int N);
void exchangeBoundaries(Vector A, int M, int N, int rank, short nb_a, short nb_u, MPI_Comm *comm);
void setOuterBoundaries(Vector A, int M, int N, int rank, int size);
void printTemperature(Vector A, int N);
void printTemperatureButFaster(Vector A, int N, int rank, int size);
void printHelper(Vector A, int N, int size);
void calc_nearby_heat_diff(Vector A, Vector B, int M, int N);

void printMat(Vector A, int M, int N, int rank, int size);

int main(int argc, char **argv) {
    
    // ------- INITIALIZATION ----------
    MPI_Init(&argc, &argv);
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);

    int rank,size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    double starttime, endtime;
    starttime = MPI_Wtime();

    int N = 768;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    int T = N * 100;

    if (N % size != 0) {
        printf("N must be divisible by the number of processes\n");
        MPI_Abort(comm, 1);        
    }

    /* ---------PARALLEL LOGIC ---------
     * - split the matrix only horizontally
     * - each process gets a couple of rows
     * - easy to pass the whole bottom/ 
     *   top row to the next/ previous process
     * 
     *  example for 4 processes:
     *  -------------------
     *  -------------------
     *  
     *  +++++++++++++++++++
     *  +++++++++++++++++++
     * 
     *  -------------------
     *  -------------------
     * 
     *  +++++++++++++++++++
     *  +++++++++++++++++++ 
     */


    int num_rows = N / size;
    Vector A = createVector(num_rows + 2, N);
    Vector B = createVector(num_rows + 2, N);
    Vector final = createVector(N, N);

    short nb_a = (rank - 1 + size) % size;
    short nb_u = (rank + 1) % size;

    int source_x = N / 4;
    int source_y = N / 4;    
    // rank where the source is located
    int source_rank = source_y / num_rows;
    // local coordinates of the source, source_x stays the same
    int local_source_y = (source_y % num_rows) + 1;
    // get index of the boundary which is more likely to have a higher temperature
    int hot_boundary = source_x < N / 2 ? 0 : N - 1;

    initializeTemperature(A, num_rows + 2, N);
    if (rank == source_rank) {
        A[IND(local_source_y, source_x)] = 273 + 60;
    }

    // ------- COMPUTATION ----------
    for (int t = 0; t < T; t++) {
        exchangeBoundaries(A, num_rows + 2, N, rank, nb_a, nb_u, &comm);

        // check if computation is necessary
        short compute = 1;
        if (rank < source_rank) {
            double val_1 = A[IND(num_rows + 1, source_x)];
            double val_2 = A[IND(num_rows + 1, hot_boundary)];
            double max_t = fmax(val_1, val_2);
            if (max_t < 273.0001) {
                // no need to compute
                compute = 0;
            }
        } else if (rank > source_rank) {
            double val_1 = A[IND(0, source_x)];
            double val_2 = A[IND(0, hot_boundary)];
            double max_t = fmax(val_1, val_2);
            if (max_t < 273.0001) {
                // no need to compute
                compute = 0;
            }
        }

        if (compute) {            
            calc_nearby_heat_diff(A, B, num_rows + 2, N);
            setOuterBoundaries(B, num_rows + 2, N, rank, size);

            Vector H = A;
            A = B;
            B = H;        
            
            // Keep the heat source temperature constant
            if (rank == source_rank) {
                A[IND(local_source_y, source_x)] = 273 + 60;
            }               
        }
        
        // collect the final vector
        if (!(t % 10000)) {
            MPI_Gather(&A[N], num_rows * N, MPI_DOUBLE, final, num_rows * N, MPI_DOUBLE, 0, comm);
            if (rank == 0) {
                printf("Step t=%d:\n", t);
                printTemperature(final, N);
                // printTemperatureButFaster(A, N, rank, size);
                printf("\n");            
            }
        }
    }

    endtime = MPI_Wtime();

    MPI_Finalize();

    // ------- OUTPUT ----------
    if (rank == 0) {
        printf("Elapsed Time: %2.4f\n", endtime - starttime);
    }

    releaseVector(B);
    releaseVector(A);
    releaseVector(final);

    return EXIT_SUCCESS;
}

void initializeTemperature(Vector A, int M, int N) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            A[IND(i, j)] = 273; // Temperature is 0°C everywhere (273 K)
        }
    }    
}

void exchangeBoundaries(Vector A, int M, int N, int rank, short nb_a, short nb_u, MPI_Comm *comm) {
    
    MPI_Request reqs[4];
    if (rank % 2 == 0) {
        // send the lower row to neighbour underneath
        MPI_Isend(&A[IND(M - 2, 0)], N, MPI_DOUBLE, nb_u, 1, *comm, &reqs[0]);
        // receive the upper row from the neighbour underneath
        MPI_Irecv(&A[IND(M - 1, 0)], N, MPI_DOUBLE, nb_u, 0, *comm, &reqs[1]);
        // send the upper row to neighbour above
        MPI_Isend(&A[IND(1, 0)], N, MPI_DOUBLE, nb_a, 1, *comm, &reqs[2]);
        // receive the lower row from the neighbour above
        MPI_Irecv(A, N, MPI_DOUBLE, nb_a, 0, *comm, &reqs[3]);
    } else {
        // receive the lower row from the neighbour above
        MPI_Irecv(A, N, MPI_DOUBLE, nb_a, 1, *comm, &reqs[0]);
        // send the upper row to neighbour above
        MPI_Isend(&A[IND(1, 0)], N, MPI_DOUBLE, nb_a, 0, *comm, &reqs[1]);
        // receive the upper row from the neighbour underneath
        MPI_Irecv(&A[IND(M - 1, 0)], N, MPI_DOUBLE, nb_u, 1, *comm, &reqs[2]);
        // send the lower row to neighbour underneath
        MPI_Isend(&A[IND(M - 2, 0)], N, MPI_DOUBLE, nb_u, 0, *comm, &reqs[3]);
    }
    MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
}

void setOuterBoundaries(Vector A, int M, int N, int rank, int size) {
    // set the outer boundaries to the values next to it to 
    // eliminate the influence of the outer boundaries   
    if (rank == 0) {
        for (int i = 0; i < N; i++) {            
            A[IND(0, i)] = A[IND(1, i)]; // top
        }
    } else if (rank == size - 1) {
        for (int i = 0; i < N; i++) {            
            A[IND(M - 1, i)] = A[IND(M - 2, i)]; // bottom
        }
    }

    for (int i = 0; i < M; i++) {        
        A[IND(i, 0)] = A[IND(i, 1)]; // left
        A[IND(i, N - 1)] = A[IND(i, N - 2)]; // right
    }    
}

void calc_nearby_heat_diff(Vector A, Vector B, int M, int N) {    
    
    for (int y = 1; y < M - 1; y++) {
        for (int x = 1; x < N - 1; x++) {
            double tc = A[IND(y, x)];
            
            double tr = A[IND(y, x + 1)];
            double tl = A[IND(y, x - 1)];
            double td = A[IND(y + 1, x)];
            double tu = A[IND(y - 1, x)];            
            
            B[IND(y, x)] = tc + 0.2 * (tr + tl + td + tu - (4 * tc));            
        }
    }
}

void printTemperature(Vector A, int N) {
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
    int sH = N / H;

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
                    max_t = (max_t < A[IND(x,y)]) ? A[IND(x,y)] : max_t;
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

void printTemperatureButFaster(Vector A, int N, int rank, int size) {
    
    int W = RESOLUTION_WIDTH;    
    // MPI_Comm comm2d;
    // MPI_Comm_dup(MPI_COMM_WORLD, &comm2d);
    if (rank == 0) {
        printf("\t");
        for (int u = 0; u < W + 2; u++) {
            printf("X");
        }
        printf("\n");
        printf("N: %d\n", N);
        printHelper(A, N, size);

        // send ready signal
        MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    } 
    else if (rank == size - 1) {
        // receive ready signal
        MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printHelper(A, N, size);

        // lower wall
        printf("\t");
        for (int l = 0; l < W + 2; l++) {
            printf("X");
        }
        printf("\n\n\n");
        fflush(NULL);
    } else {
        // receive ready signal
        MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printHelper(A, N, size);

        // send ready signal
        MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
}

void printHelper(Vector A, int N, int size) {
    const char *colors = " .-:=+*^X#%@";
    const int numColors = 12;

    // boundaries for temperature (for simplicity hard-coded)
    const double max = 273 + 30;
    const double min = 273 + 0;

    // set the 'render' resolution
    int W = RESOLUTION_WIDTH;
    int H = RESOLUTION_HEIGHT / size;

    // step size in each dimension
    int M = N / size;
    int sW = N / W;
    int sH = M / H;

    // room
    for (int i = 1; i < H - 1; i++) {
        // left wall
        printf("\tX");
        // actual room
        for (int j = 0; j < W; j++) {
            // get max temperature in this tile
            double max_t = 0;
            for (int x = sH * i; x < sH * i + sH; x++) {
                for (int y = sW * j; y < sW * j + sW; y++) {
                    max_t = (max_t < A[IND(x,y)]) ? A[IND(x,y)] : max_t;
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
        fflush(NULL);
    }
}

Vector createVector(int M, int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * M * N);
}

void releaseVector(Vector m) { free(m); }

void printMat(Vector A, int M, int N, int rank, int size) {
    // do not interfere with printf's from other processes
    if (rank) {
        MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("printMat from rank %d\n", rank);
    for (int y = 0; y < M; y++) {
        for (int x = 0; x < N; x++)
            printf("%5.0lf ", A[IND(y, x)]);
        printf("\n");
    }
    printf("\n");    
    fflush(NULL);
    
    if (rank < size - 1) {
        MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }
}
