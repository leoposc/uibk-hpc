#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common2d_int.h"

void calc_nearby_heat_diff_int(Vector A, Vector B, int M, int N);
void calc_nearby_heat_diff(Vector A, Vector B, int M, int N, MPI_Request* reqs);
void exchangeBoundaries(Vector A, int M, int N, int rank, short nb_a, short nb_u, MPI_Comm* comm,
                        MPI_Request* reqs);
void setOuterBoundaries(Vector A, int M, int N, int rank, int size);

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
	if(rank == source_rank) {
		A[IND(local_source_y, source_x)] = double_to_int(273 + 60);
	}

	// ------- COMPUTATION ----------
	for(int t = 0; t < T; t++) {
		MPI_Request reqs[4];
		exchangeBoundaries(A, num_rows + 2, N, rank, nb_a, nb_u, &comm, reqs);

		// check if computation is necessary
		short compute = 1;
		if(rank < source_rank) {
			double val_1 = A[IND(num_rows + 1, source_x)];
			double val_2 = A[IND(num_rows + 1, hot_boundary)];
			double max_t = fmax(val_1, val_2);
			if(max_t < double_to_int(273.0001)) {
				// no need to compute
				compute = 0;
			}
		} else if(rank > source_rank) {
			double val_1 = A[IND(0, source_x)];
			double val_2 = A[IND(0, hot_boundary)];
			double max_t = fmax(val_1, val_2);
			if(max_t < double_to_int(273.0001)) {
				// no need to compute
				compute = 0;
			}
		}

		if(compute) {
			calc_nearby_heat_diff(A, B, num_rows + 2, N, reqs);
			setOuterBoundaries(B, num_rows + 2, N, rank, size);

			Vector H = A;
			A = B;
			B = H;

			// Keep the heat source temperature constant
			if(rank == source_rank) {
				A[IND(local_source_y, source_x)] = double_to_int(273 + 60);
			}
		} else {
			MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
		}

		// collect the final vector
		if(!(t % 10000)) {
			MPI_Gather(&A[N], num_rows * N, MPI_DOUBLE, final, num_rows * N, MPI_DOUBLE, 0, comm);
			if(rank == 0) {
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
	if(rank == 0) {
		printf("$!N_%d{%d, %2.4f}\n", N, size, endtime - starttime);
	}

	releaseVector(B);
	releaseVector(A);
	releaseVector(final);

	return EXIT_SUCCESS;
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

void calc_nearby_heat_diff(Vector A, Vector B, int M, int N, MPI_Request* reqs) {

	for(int y = 2; y < M - 2; y++) {
		for(int x = 1; x < N - 1; x++) {
			u_int64_t tc = A[IND(y, x)];

			u_int64_t tr = A[IND(y, x + 1)];
			u_int64_t tl = A[IND(y, x - 1)];
			u_int64_t td = A[IND(y + 1, x)];
			u_int64_t tu = A[IND(y - 1, x)];

      B[IND(y, x)] = tc + ((tr + tl + td + tu - (tc << 2)) >> 2);
		}
	}

	// compute ghost cells
	MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
	for(int x = 1; x < N - 1; x++) {
		u_int64_t tc = A[IND(1, x)];

		u_int64_t tr = A[IND(1, x + 1)];
		u_int64_t tl = A[IND(1, x - 1)];
		u_int64_t td = A[IND(2, x)];
		u_int64_t tu = A[IND(0, x)];

    B[IND(1, x)] = tc + ((tr + tl + td + tu - (tc << 2)) >> 2);

		tc = A[IND(M - 2, x)];

		tr = A[IND(M - 2, x + 1)];
		tl = A[IND(M - 2, x - 1)];
		td = A[IND(M - 1, x)];
		tu = A[IND(M - 3, x)];

    B[IND(M-2, x)] = tc + ((tr + tl + td + tu - (tc << 2)) >> 2);
	}
}

void exchangeBoundaries(Vector A, int M, int N, int rank, short nb_a, short nb_u, MPI_Comm* comm,
                        MPI_Request* reqs) {

	if(rank % 2 == 0) {
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
	// MPI_Waitall(4, reqs, MPI_STATUSES_IGNORE);
}

void setOuterBoundaries(Vector A, int M, int N, int rank, int size) {
	// set the outer boundaries to the values next to it to
	// eliminate the influence of the outer boundaries
	if(rank == 0) {
		for(int i = 0; i < N; i++) {
			A[IND(0, i)] = A[IND(1, i)]; // top
		}
	} else if(rank == size - 1) {
		for(int i = 0; i < N; i++) {
			A[IND(M - 1, i)] = A[IND(M - 2, i)]; // bottom
		}
	}

	for(int i = 0; i < M; i++) {
		A[IND(i, 0)] = A[IND(i, 1)];         // left
		A[IND(i, N - 1)] = A[IND(i, N - 2)]; // right
	}
}
