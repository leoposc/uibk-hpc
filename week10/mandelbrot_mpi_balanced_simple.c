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

void calcMandelbrot(uint8_t* image, int offset_x, int size_x, int size_y, int R) {

	const float left = -2.5, right = 1;
	const float bottom = -1, top = 1;

	for(int abs_pixel_y = 0; abs_pixel_y < size_y; abs_pixel_y++) {
		// scale y pixel into mandelbrot coordinate system
		const float cy = (abs_pixel_y / (float) size_y) * (top - bottom) + bottom;
		for(int abs_pixel_x = offset_x; abs_pixel_x < size_x; abs_pixel_x += R) {
			// scale x pixel into mandelbrot coordinate system
			const float cx = (abs_pixel_x / (float) size_x) * (right - left) + left;
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
			image[IND(abs_pixel_y, abs_pixel_x, size_y, size_x, channel++)] = (uint8_t)(red * UINT8_MAX);
			image[IND(abs_pixel_y, abs_pixel_x, size_y, size_x, channel++)] = (uint8_t)(green * UINT8_MAX);
			image[IND(abs_pixel_y, abs_pixel_x, size_y, size_x, channel++)] = (uint8_t)(blue * UINT8_MAX);
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

	int size_x = DEFAULT_SIZE_X;
	int size_y = DEFAULT_SIZE_Y;

	if(argc == 3) {
		size_x = atoi(argv[1]);
		size_y = atoi(argv[2]);
	} else {
		printf("No arguments given, using default size\n");
	}

	// number of ranks
	const int R = size;

	
	// offsets
	const int offset_x = rank;

	uint8_t* image = malloc(NUM_CHANNELS * size_x * size_y * sizeof(uint8_t));

	// inint image with zeros

	struct timeval start, end, mid;

	gettimeofday(&start, NULL);

	// comptue the local image
	calcMandelbrot(image, offset_x, size_x, size_y, R);

	gettimeofday(&mid, NULL);
	double time_before_collect = (mid.tv_sec + mid.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	printf("%d %f\n", rank, time_before_collect);

	// collect all the local images on rank 0
	uint8_t* final = NULL;
	if (rank == 0) {
 		final = malloc(NUM_CHANNELS * size_x * size_y * sizeof(uint8_t));
	}


	// MPI_Gather(image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, image,
	// 	size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, 0, comm);
	// reduce the image on rank 0
	MPI_Reduce(image, final, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, MPI_SUM, 0, comm);

	// measure the exeuction time
	gettimeofday(&end, NULL);
	double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	double maxtimeElapsed = 0;
	MPI_Reduce(&timeElapsed, &maxtimeElapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
	if (rank == 0) {	
		printf("Mandelbrot set calculation for %dx%d took: %f seconds.\n", size_x, size_y, timeElapsed);
	}


	// export the collected image as .png
	if (rank == 0) {
		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi.png", size_x, size_y, NUM_CHANNELS, image, stride_bytes);
	}

	free(image);

	if (rank == 0) {
		free(final);
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