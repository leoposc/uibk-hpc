#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
	int rank;
	int size;
	int sBuf[2] = { 1, 2 };
	int rBuf[2];
	MPI_Status status;
	MPI_Datatype newType;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// Enough tasks?
	if(size < 2) {
		printf("This test needs at least 2 processes ! \n");
		MPI_Finalize();
		return 1;
	}

	// Say hello
	printf("Hello, I am rank %d of %d processes. \n", rank, size);

	//) Create a datatype
	MPI_Type_contiguous(2, MPI_INT, &newType);
	MPI_Type_commit(&newType);

	// 2) Use MPI Sendrecv to perform a ring communication
	// FIXED: replace MPI_BYTE with MPI_INT 
	// FIXED: replace (rank = 1 + size) with (rank - 1 + size)
	MPI_Sendrecv(sBuf, 1, newType, (rank + 1) % size, 123, rBuf, sizeof(int) * 2, MPI_INT,
	             (rank - 1 + size) % size, 123, MPI_COMM_WORLD, &status);

	// 3) Use MPI Send and MPI Recv to perform a ring communication
	// FIXED: replace MPI_BYTE with MPI_INT
	// FIXED: replace (rank = 1 + size) with (rank - 1 + size)
	// FIXED: corrctly orchestate MPI_Send and MPI_Recv to avoid deadlock
	if (rank % 2) {
		MPI_Send(sBuf, 1, newType, (rank + 1) % size, 456, MPI_COMM_WORLD);
		MPI_Recv(rBuf, sizeof(int) * 2, MPI_INT, (rank - 1 + size) % size, 456, MPI_COMM_WORLD,
				&status);
	} else {
		MPI_Recv(rBuf, sizeof(int) * 2, MPI_INT, (rank - 1 + size) % size, 456, MPI_COMM_WORLD,
	         &status);
		MPI_Send(sBuf, 1, newType, (rank + 1) % size, 456, MPI_COMM_WORLD);
	}

	// Say bye bye
	printf("Signing off, rank %d. \n", rank);

	// FIXED: added this line to prevent memory leaks
	MPI_Type_free(&newType);

	MPI_Finalize();
	return 0;
}
