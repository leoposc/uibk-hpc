#ifndef COMMON1D
#define COMMON1D

#define RESOLUTION 120

typedef double value_t;

// -- vector utilities --
typedef value_t* Vector;

Vector createVector(int N);
void releaseVector(Vector m);
void printTemperature(Vector m, int N);

Vector createVector(int N) {
	// create data and index vector
	return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) {
	free(m);
}

void printTemperature(Vector m, int N) {
	const char* colors = " .-:=+*^X#%@";
	const int numColors = 12;

	// boundaries for temperature (for simplicity hard-coded)
	const value_t max = 273 + 30;
	const value_t min = 273 + 0;

	// set the 'render' resolution
	int W = RESOLUTION;

	// step size in each dimension
	int sW = N / W;

	// room
	// left wall
	printf("X");
	// actual room
	for(int i = 0; i < W; i++) {
		// get max temperature in this tile
		value_t max_t = 0;
		for(int x = sW * i; x < sW * i + sW; x++) {
			max_t = (max_t < m[x]) ? m[x] : max_t;
		}
		value_t temp = max_t;

		// pick the 'color'
		int c = ((temp - min) / (max - min)) * numColors;
		c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

		// print the average temperature
		printf("%c", colors[c]);
	}
	// right wall
	printf("X");
}

#endif /* COMMON1D */