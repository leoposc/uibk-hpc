#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/common1d.h"

int main(int argc, char** argv) {
	// 'parsing' optional input parameter = problem size
	int N = 2000;
	if(argc > 1) {
		N = atoi(argv[1]);
	}
	int T = N * 500;

	// ---------- setup ----------
	MPI_Init(&argc, &argv);
	MPI_Comm comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &comm);
	int size, rank;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);
	double starttime, endtime;
	starttime = MPI_Wtime();

	if(rank == 0)
		printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);

	short left_rank = rank - 1;
	short right_rank = rank + 1;
	int local_N = N / size;

	// create a buffer for storing temperature fields
	Vector A = createVector(local_N);
	Vector B = createVector(local_N);
	Vector merged = createVector(N);

	// set up initial conditions in A
	for(int i = 0; i < local_N; i++) {
		A[i] = 273; // temperature is 0° C everywhere (273 K)
	}

	// and there is a heat source in one corner
	int source_x = N / 4;
	int offset = local_N * rank;
	int source_x_offset = source_x - offset;
	if(0 <= source_x_offset && source_x_offset < local_N) {
		A[source_x_offset] = 273 + 60;
	}

	// ---------- compute ----------
	double left_temp_recv = 0.0;
	double right_temp_recv = 0.0;

	// for each time step ..
	for(int t = 0; t < T; t++) {

		// ------- send data ---------
		if(0 <= left_rank) {
			// ...we send left boundary to left neighbour
			double left_temp_send = A[0];
			MPI_Send(&left_temp_send, 1, MPI_DOUBLE, left_rank, rank, comm);
		}

		if(right_rank < size) {
			// ...we send right boundary to right neighbour
			double right_temp_send = A[local_N - 1];
			MPI_Send(&right_temp_send, 1, MPI_DOUBLE, right_rank, rank, comm);
		}

		// ------ receive data --------

		// .. we wait for boundary values from right neighbour
		if(0 <= left_rank) {
			// ...we receive right boundary from left neighbour
			MPI_Recv(&left_temp_recv, 1, MPI_DOUBLE, left_rank, left_rank, comm, MPI_STATUS_IGNORE);
		} else {
			// ...we handle very left boundary
			left_temp_recv = A[0];
		}

		if(right_rank < size) {
			// ...we receive left boundary from right neighbour
			MPI_Recv(&right_temp_recv, 1, MPI_DOUBLE, right_rank, right_rank, comm,
			         MPI_STATUS_IGNORE);
		} else {
			// ...we handle very right boundary
			right_temp_recv = A[local_N - 1];
		}

		// .. we propagate the temperature
		for(long long i = 0; i < local_N; i++) {
			// center stays constant (the heat is still on)
			if(i == source_x_offset && 0 <= source_x_offset && source_x_offset < local_N) {
				A[source_x_offset] = 273 + 60;
				continue;
			}

			// get temperature at current position
			value_t tc = A[i];

			// get temperatures of adjacent cells
			value_t tl = (i != 0) ? A[i - 1] : left_temp_recv;
			value_t tr = (i != local_N - 1) ? A[i + 1] : right_temp_recv;

			// compute new temperature at current position
			B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
		}

		// swap matrices (just pointers, not content)
		Vector H = A;
		A = B;
		B = H;

		// collect the final vector
		MPI_Gather(A, local_N, MPI_DOUBLE, merged, local_N, MPI_DOUBLE, 0, comm);
		if(rank == 0) {
			// show intermediate step
			if(!(t % 10000)) {
				printf("Step t=%d:\t", t);
				printTemperature(merged, N);
				printf("\n");
			}
		}
	}

	releaseVector(B);

	// ---------- check ----------
	int success = 1;
	if(rank == 0) {
		printf("Final:\t\t");
		printTemperature(merged, N);
		printf("\n");

		for(long long i = 0; i < N; i++) {
			value_t temp = merged[i];
			if(273 <= temp && temp <= 273 + 60) continue;
			success = 0;
			break;
		}

		printf("Verification: %s\n", (success) ? "OK" : "FAILED");

		// ------ time measurement -----
		endtime = MPI_Wtime();
		printf("time elapsed: %f\n", endtime - starttime);
	}

	// ---------- cleanup ----------
	releaseVector(A);
	releaseVector(merged);

	// done
	MPI_Finalize();
	if(rank == 0) {
		return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
	} else {
		return EXIT_SUCCESS;
	}
}
