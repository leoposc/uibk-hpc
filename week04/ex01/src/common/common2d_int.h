#ifndef COMMON2D
#define COMMON2D

#define RESOLUTION_WIDTH 50
#define RESOLUTION_HEIGHT 50

#define IND(y, x) ((y) * (N) + (x))

typedef u_int64_t value_t;
typedef value_t* Vector;

double int_to_double(u_int64_t val_int);
u_int64_t double_to_int(double val_double);
Vector createVector(int N, int M);
void releaseVector(Vector m);
void printMat(Vector A, int M, int N, int rank, int size);
void printTemperature(Vector A, int N);
void printTemperatureButFaster(Vector A, int N, int rank, int size);
void printHelper(Vector A, int N, int size);
void printMat(Vector A, int M, int N, int rank, int size);

void printTemperature(Vector A, int N) {
	const char* colors = " .-:=+*^X#%@";
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
	for(int u = 0; u < W + 2; u++) {
		printf("X");
	}
	printf("\n");
	// room
	for(int i = 0; i < H; i++) {
		// left wall
		printf("\tX");
		// actual room
		for(int j = 0; j < W; j++) {
			// get max temperature in this tile
			double max_t = 0;
			for(int x = sH * i; x < sH * i + sH; x++) {
				for(int y = sW * j; y < sW * j + sW; y++) {
					max_t = (max_t < int_to_double(A[IND(x, y)])) ? int_to_double(A[IND(x, y)]) : max_t;
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
	for(int l = 0; l < W + 2; l++) {
		printf("X");
	}
	printf("\n");
}

void printTemperatureButFaster(Vector A, int N, int rank, int size) {

	int W = RESOLUTION_WIDTH;
	// MPI_Comm comm2d;
	// MPI_Comm_dup(MPI_COMM_WORLD, &comm2d);
	if(rank == 0) {
		printf("\t");
		for(int u = 0; u < W + 2; u++) {
			printf("X");
		}
		printf("\n");
		printf("N: %d\n", N);
		printHelper(A, N, size);

		// send ready signal
		MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	} else if(rank == size - 1) {
		// receive ready signal
		MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		printHelper(A, N, size);

		// lower wall
		printf("\t");
		for(int l = 0; l < W + 2; l++) {
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
	const char* colors = " .-:=+*^X#%@";
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
	for(int i = 1; i < H - 1; i++) {
		// left wall
		printf("\tX");
		// actual room
		for(int j = 0; j < W; j++) {
			// get max temperature in this tile
			double max_t = 0;
			for(int x = sH * i; x < sH * i + sH; x++) {
				for(int y = sW * j; y < sW * j + sW; y++) {
					max_t = (max_t < int_to_double(A[IND(x, y)])) ? int_to_double(A[IND(x, y)]) : max_t;
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

void printMat(Vector A, int M, int N, int rank, int size) {
	// do not interfere with printf's from other processes
	if(rank) {
		MPI_Recv(NULL, 0, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	printf("printMat from rank %d\n", rank);
	for(int y = 0; y < M; y++) {
		for(int x = 0; x < N; x++)
			printf("%5.0lf ", int_to_double(A[IND(y, x)]));
		printf("\n");
	}
	printf("\n");
	fflush(NULL);

	if(rank < size - 1) {
		MPI_Send(NULL, 0, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	}
}

Vector createVector(int M, int N) {
	// create data and index vector
	return malloc(sizeof(value_t) * M * N);
}

void releaseVector(Vector m) {
	free(m);
}

void initializeTemperature(Vector A, int M, int N) {
	for(int i = 0; i < M; i++) {
		for(int j = 0; j < N; j++) {
			A[IND(i, j)] = double_to_int(273); // Temperature is 0°C everywhere (273 K)
		}
	}
}

double int_to_double(u_int64_t val_int) {
	return val_int / 4294967296.0;
}

u_int64_t double_to_int(double val_double) {
	return round(val_double * 4294967296);
}

#endif /* COMMON2D */