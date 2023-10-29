#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common2d_int.h"

void calc_nearby_heat_diff_int(Vector A, Vector B, int M, int N);
void exchangeBoundaries(Vector A, int M, int N, int rank, int size, MPI_Comm* comm);
void setOuterBoundaries(Vector A, int M, int N, int rank, int size);
void calc_nearby_heat_diff(Vector A, Vector B, int M, int N);

int main(int argc, char** argv) {

	// ------- INITIALIZATION ----------
	MPI_Init(&argc, &argv);
	MPI_Comm comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &comm);

	int rank, size;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	double starttime, endtime;
	starttime = MPI_Wtime();
	printf("started");

	int N = 768;
	if(argc > 1) {
		N = atoi(argv[1]);
	}
	int T = N * 100;

	if(N % size != 0) {
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

	int source_x = N / 4;
	int source_y = N / 4;
	// rank where the source is located
	int source_rank = source_y / num_rows;
	// local coordinates of the source, source_x stays the same
	int local_source_y = (source_y % num_rows) + 1;
	// get index of the boundary which is more likely to have a higher temperature

	initializeTemperature(A, num_rows + 2, N);
  printf("a: %ld\n", A[IND(4,4)]);
	if(rank == source_rank) {
		A[IND(local_source_y, source_x)] = double_to_int(273 + 60);
	}


	double test = 273 + M_PI;
	printf("normal calc: %lf\n", ( test + 0.25 * (test + test + 300 + test - (4 * test))));
	printf("before conversion: %f\n", test);
	u_int64_t converted = double_to_int(test);

  converted = converted + ((converted + converted + double_to_int(300) + converted - (converted << 2)) >> 2);
	printf("int val: %ld\n", converted);
	test = int_to_double(converted);
	printf("after conversion: %f\n", test);
	// ------- COMPUTATION ----------
	for(int t = 0; t < T; t++) {

		exchangeBoundaries(A, num_rows + 2, N, rank, size, &comm);
		// check if computation is necessary

		calc_nearby_heat_diff_int(A, B, num_rows + 2, N);
		setOuterBoundaries(B, num_rows + 2, N, rank, size);

		Vector H = A;
		A = B;
		B = H;

		// Keep the heat source temperature constant
		if(rank == source_rank) {
			A[IND(local_source_y, source_x)] = double_to_int(273 + 60);
		}

		// collect the final vector
    // printMat(A, num_rows + 2, N, 0, size);
		if(!(t % 10000)) {
			MPI_Gather(&A[N], num_rows * N, MPI_DOUBLE, final, num_rows * N, MPI_DOUBLE, 0, comm);
			if(rank == 0) {
				printf("Step t=%d:\n", t);
				printTemperature(final, N);
				printf("\n");
			}
		}
	}

	endtime = MPI_Wtime();

	MPI_Finalize();

	// ------- OUTPUT ----------
	if(rank == 0) {
		printf("$!N_%d{%d, %2.4f}\n", N, size, endtime - starttime);
	}

	releaseVector(B);
	releaseVector(A);
	releaseVector(final);

	return EXIT_SUCCESS;
}

void exchangeBoundaries(Vector A, int M, int N, int rank, int size, MPI_Comm* comm) {
	short neighbour_above = (rank - 1 + size) % size;
	short neighbour_underneath = (rank + 1) % size;
	// printf("Rank %d is sending.....", rank);
	if(rank == 0) {
		// send the lower row to neighbour underneath
		MPI_Send(&A[IND(M - 2, 0)], N, MPI_DOUBLE, neighbour_underneath, 1, *comm);
		// receive the upper row from the neighbour underneath
		MPI_Recv(&A[IND(M - 1, 0)], N, MPI_DOUBLE, neighbour_underneath, 0, *comm,
		         MPI_STATUS_IGNORE);
	} else if(rank == size - 1) {
		// receive the lower row from the neighbour above
		MPI_Recv(A, N, MPI_DOUBLE, neighbour_above, 1, *comm, MPI_STATUS_IGNORE);
		// send the upper row to neighbour above
		MPI_Send(&A[IND(1, 0)], N, MPI_DOUBLE, neighbour_above, 0, *comm);
	} else if(rank % 2 == 0) {
		// send the lower row to neighbour underneath
		MPI_Send(&A[IND(M - 2, 0)], N, MPI_DOUBLE, neighbour_underneath, 1, *comm);
		// receive the upper row from the neighbour underneath
		MPI_Recv(&A[IND(M - 1, 0)], N, MPI_DOUBLE, neighbour_underneath, 0, *comm,
		         MPI_STATUS_IGNORE);
		// send the upper row to neighbour above
		MPI_Send(&A[IND(1, 0)], N, MPI_DOUBLE, neighbour_above, 1, *comm);
		// receive the lower row from the neighbour above
		MPI_Recv(A, N, MPI_DOUBLE, neighbour_above, 0, *comm, MPI_STATUS_IGNORE);
	} else {
		// receive the lower row from the neighbour above
		MPI_Recv(A, N, MPI_DOUBLE, neighbour_above, 1, *comm, MPI_STATUS_IGNORE);
		// send the upper row to neighbour above
		MPI_Send(&A[IND(1, 0)], N, MPI_DOUBLE, neighbour_above, 0, *comm);
		// receive the upper row from the neighbour underneath
		MPI_Recv(&A[IND(M - 1, 0)], N, MPI_DOUBLE, neighbour_underneath, 1, *comm,
		         MPI_STATUS_IGNORE);
		// send the lower row to neighbour underneath
		MPI_Send(&A[IND(M - 2, 0)], N, MPI_DOUBLE, neighbour_underneath, 0, *comm);
	}
	// printf(".........Rank %d received everything\n", rank);
}

void setOuterBoundaries(Vector A, int M, int N, int rank, int size) {
	// set the outer boundaries to the values next to it to
	// eliminate the influence of the outer boundaries
	if(rank == 0) {
		for(int i = 0; i < N; i++) {
			// top
			A[IND(0, i)] = A[IND(1, i)];
		}
	} else if(rank == size - 1) {
		for(int i = 0; i < N; i++) {
			// bottom
			A[IND(M - 1, i)] = A[IND(M - 2, i)];
		}
	}

	// for (int i = 0; i < N; i++) {
	//     // top
	//     A[IND(0, i)] = A[IND(1, i)];
	//     // bottom
	//     A[IND(M - 1, i)] = A[IND(M - 2, i)];
	// }
	for(int i = 0; i < M; i++) {
		// left
		A[IND(i, 0)] = A[IND(i, 1)];
		// right
		A[IND(i, N - 1)] = A[IND(i, N - 2)];
	}
}

void calc_nearby_heat_diff_int(Vector A, Vector B, int M, int N) {

	for(int y = 1; y < M - 1; y++) {
		for(int x = 1; x < N - 1; x++) {
			u_int64_t tc = A[IND(y, x)];

			u_int64_t tr = A[IND(y, x + 1)];
			u_int64_t tl = A[IND(y, x - 1)];
			u_int64_t td = A[IND(y + 1, x)];
			u_int64_t tu = A[IND(y - 1, x)];

      B[IND(y, x)] = tc + ((tr + tl + td + tu - (tc << 2)) >> 2);
		}
	}
}

void calc_nearby_heat_diff(Vector A, Vector B, int M, int N) {

	for(int y = 1; y < M - 1; y++) {
		for(int x = 1; x < N - 1; x++) {
			double tc = A[IND(y, x)];

			double tr = A[IND(y, x + 1)];
			double tl = A[IND(y, x - 1)];
			double td = A[IND(y + 1, x)];
			double tu = A[IND(y - 1, x)];

			B[IND(y, x)] = tc + 0.2 * (tr + tl + td + tu - (4 * tc));
		}
	}
}
