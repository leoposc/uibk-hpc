#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Include that allows to print result as an image
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define DEFAULT_SIZE_X 1280
#define DEFAULT_SIZE_Y 720

// RGB image will hold 3 color channels
#define NUM_CHANNELS 3
// max iterations cutoff
#define MAX_ITER 10000

#define IND(Y, X, SIZE_Y, SIZE_X, CHANNEL) ((Y) * (SIZE_X) * (NUM_CHANNELS) + (X) * (NUM_CHANNELS) + (CHANNEL))

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B);

void calcMandelbrot(uint8_t* local_image, int offset_x, int offset_y, int size_x, int size_y,
		int total_size_x, int total_size_y) {
	const float left = -2.5, right = 1;
	const float bottom = -1, top = 1;

	for(int abs_pixel_y = offset_y; abs_pixel_y < offset_y + size_y; abs_pixel_y++) {
		// scale y pixel into mandelbrot coordinate system
		const float cy = (abs_pixel_y / (float) total_size_y) * (top - bottom) + bottom;
		for(int abs_pixel_x = offset_x; abs_pixel_x < offset_x + size_x; abs_pixel_x++) {
			// scale x pixel into mandelbrot coordinate system
			const float cx = (abs_pixel_x / (float) total_size_x) * (right - left) + left;
			float x = 0;
			float y = 0;
			int numIterations = 0;

			// Check if the distance from the origin becomes 
			// greater than 2 within the max number of iterations.
			while((x * x + y * y <= 2 * 2) && (numIterations < MAX_ITER)) {
				float x_tmp = x * x - y * y + cx;
				y = 2 * x * y + cy;
				x = x_tmp;
				numIterations += 1;
			}
			
			// Normalize iteration and write it to pixel position			
			double value = fabs((numIterations / (float)MAX_ITER)) * 200;

			double red = 0;
			double green = 0;
			double blue = 0;

			HSVToRGB(value, 1.0, 1.0, &red, &green, &blue);

			int channel = 0;
			local_image[IND(abs_pixel_y - offset_y, abs_pixel_x - offset_x, size_y, size_x, channel++)] = (uint8_t)(red * UINT8_MAX);
			local_image[IND(abs_pixel_y - offset_y, abs_pixel_x - offset_x, size_y, size_x, channel++)] = (uint8_t)(green * UINT8_MAX);
			local_image[IND(abs_pixel_y - offset_y, abs_pixel_x - offset_x, size_y, size_x, channel++)] = (uint8_t)(blue * UINT8_MAX);
		}
	}
}

int main(int argc, char** argv) {

    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);

	int totalSizeX = DEFAULT_SIZE_X;
	int totalSizeY = DEFAULT_SIZE_Y;

	if(argc == 3) {
		totalSizeX = atoi(argv[1]);
		totalSizeY = atoi(argv[2]);
	} else {
		printf("No arguments given, using default size\n");
	}

	// number of ranks
	const int R = size;

	// size of a slab
	const int size_x = totalSizeX;
	const int size_y = totalSizeY / R;

	// offsets
	const int offset_x = 0;
	const int offset_y = rank * size_y;

	uint8_t* local_image = malloc(NUM_CHANNELS * size_x * size_y * sizeof(uint8_t));

	struct timeval start, end;
	if (rank == 0) {
		gettimeofday(&start, NULL);
	}

	// comptue the local image
	calcMandelbrot(local_image, offset_x, offset_y, size_x, size_y, totalSizeX, totalSizeY);

	// measure the exeuction time
	if (rank == 0) {
		gettimeofday(&end, NULL);
		double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
		printf("Mandelbrot set calculation for %dx%d took: %f seconds.\n", totalSizeX, totalSizeY, timeElapsed);
	}

	// collect all the local images on rank 0
	uint8_t* image = NULL;
	if (rank == 0) {
 		image = malloc(NUM_CHANNELS * totalSizeX * totalSizeY * sizeof(uint8_t));
	}
	MPI_Gather(local_image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, image,
		size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, 0, comm);

	// export the collected image as .png
	if (rank == 0) {
		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi.png", totalSizeX, totalSizeY, NUM_CHANNELS, image, stride_bytes);
	}

	free(local_image);

	if (rank == 0) {
		free(image);
	}
	
	MPI_Finalize();

	return EXIT_SUCCESS;
}

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B) {
	if (H >= 1.00) {
		V = 0.0;
		H = 0.0;
	}

	double step = 1.0/6.0;
	double vh = H/step;

	int i = (int)floor(vh);

	double f = vh - i;
	double p = V*(1.0 - S);
	double q = V*(1.0 - (S*f));
	double t = V*(1.0 - (S*(1.0 - f)));

	switch (i) {
		case 0:
			{
				*R = V;
				*G = t;
				*B = p;
				break;
			}
		case 1:
			{
				*R = q;
				*G = V;
				*B = p;
				break;
			}
		case 2:
			{
				*R = p;
				*G = V;
				*B = t;
				break;
			}
		case 3:
			{
				*R = p;
				*G = q;
				*B = V;
				break;
			}
		case 4:
			{
				*R = t;
				*G = p;
				*B = V;
				break;
			}
		case 5:
			{
				*R = V;
				*G = p;
				*B = q;
				break;
			}
	}
}