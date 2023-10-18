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
void exchangeBoundaries(Vector A, int M, int N, int rank, int size, const MPI_Comm& comm);
void updateInterior(Vector A, Vector B, int N);
void printTemperature(Vector A, int N);
void calc_nearby_heat_diff(Vector A, Vector B, int N);

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

    int N = 200;
    if (argc > 1) {
        N = atoi(argv[1]);
    }
    int T = N * 10;

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
    Vector A = createVector(num_rows, N);
    Vector B = createVector(num_rows, N);

    int source_x = N / 4;
    int source_y = N / 4;    
    // rank where the source is located
    int source_rank = source_y / num_rows;
    // local coordinates of the source, source_x stays the same
    int local_source_y = source_y % num_rows;


    initializeTemperature(A, num_rows, N);
    if (rank == source_rank) {
        A[IND(local_source_y, source_x)] = 273 + 60;
    }

    // ------- COMPUTATION ----------
    for (int t = 0; t < T; t++) {
        exchangeBoundaries(A, num_rows, rank, size);
        calc_nearby_heat_diff(A, B, num_rows);
        updateInterior(A, B, num_rows);

        // Keep the heat source temperature constant
        if (rank == source_rank) {
            A[IND(local_source_y, source_x)] = 273 + 60;
        }
    }

    double endtime = MPI_WTime();

    MPI_Finalize();

    // ------- OUTPUT ----------
    if (rank == 0) {
        printf("Elapsed Time: %2.4f\n", endtime - starttime);
    }

    releaseVector(B);
    releaseVector(A);

    return EXIT_SUCCESS;
}

void initializeTemperature(Vector A, int M, int N) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            A[IND(i, j)] = 273; // Temperature is 0°C everywhere (273 K)
        }
    }    
}

void exchangeBoundaries(Vector A, int M, int N, int rank, int size, const MPI_Comm& comm) {
    short neighbour_above = rank - 1;
    short neighbour_underneath = rank + 1;

    if (rank % 2) {
        // send the upper row to neighbour above
        MPI_Send(A, N, MPI_DOUBLE, neighbour_above, rank, comm, MPI_STATUS_IGNORE);
        // receive the lower row from the neighbour above
        MPI_Recv(A, N, MPI_DOUBLE, neighbour_above, neighbour_above, comm, MPI_STATUS_IGNORE);
        // send the lower row to neighbour underneath
        MPI_Send(&A[IND(M, 0)], N, MPI_DOUBLE, neighbour_underneath, rank, comm, MPI_STATUS_IGNORE);
        // receive the upper row from the neighbour underneath
        MPI_Recv(&A[IND(M, 0)], N, MPI_DOUBLE, neighbour_underneath, rank, comm, MPI_STATUS_IGNORE);
    } else {
        // receive the upper row from the neighbour underneath
        MPI_Recv(&A[IND(M, 0)], N, MPI_DOUBLE, neighbour_underneath, rank, comm, MPI_STATUS_IGNORE);
        // send the lower row to neighbour underneath
        MPI_Send(&A[IND(M, 0)], N, MPI_DOUBLE, neighbour_underneath, rank, comm, MPI_STATUS_IGNORE);
        // receive the lower row from the neighbour above
        MPI_Recv(A, N, MPI_DOUBLE, neighbour_above, neighbour_above, comm, MPI_STATUS_IGNORE);
        // send the upper row to neighbour above
        MPI_Send(A, N, MPI_DOUBLE, neighbour_above, rank, comm, MPI_STATUS_IGNORE);
    }

}


Vector createVector(int M, int N) {
  // create data and index vector
  return malloc(sizeof(value_t) * M * N);
}

void releaseVector(Vector m) { free(m); }