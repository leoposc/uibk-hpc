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
#define RANK_ACC_MULTIPLYER 20

#define IND(Y, X, SIZE_Y, SIZE_X, CHANNEL) ((Y) * (SIZE_X) * (NUM_CHANNELS) + (X) * (NUM_CHANNELS) + (CHANNEL))

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B);

void estimateMandelBrotComplexity(uint32_t* complexity, int size_x, int size_y, int total_size_x, int total_size_y) {
	const float left = -2.5, right = 1;
	const float bottom = -1, top = 1;

	for(int abs_pixel_y = 0; abs_pixel_y < size_y; abs_pixel_y++) {
    uint32_t numSlabIterations = 0;
		// scale y pixel into mandelbrot coordinate system
		const float cy = (abs_pixel_y / (float) total_size_y) * (top - bottom) + bottom;
		for(int abs_pixel_x = 0; abs_pixel_x < size_x; abs_pixel_x++) {
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
        numSlabIterations++;
			}
		}
		complexity[abs_pixel_y] = numSlabIterations;
	}
}

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
  
  // calculate load on slabs
	uint32_t* complexity = malloc(R*RANK_ACC_MULTIPLYER * sizeof(uint32_t));
  estimateMandelBrotComplexity(complexity, R*RANK_ACC_MULTIPLYER, R*RANK_ACC_MULTIPLYER, R*RANK_ACC_MULTIPLYER, R*RANK_ACC_MULTIPLYER);
  
  uint32_t sum = 0;
  for (int i = 0; i < R*RANK_ACC_MULTIPLYER; i++) {
    sum += complexity[i];
  }
  
  
  // distribute slabs so that each gets roughly the same complexity
  uint32_t slices[R];
  int j = 0;
  int slice_sum = 0;
  for (int i = 0; i < R; i++) {
    uint32_t local_sum = 0;
    slices[i] = 0;
    while (local_sum <= sum/R && slice_sum < R * RANK_ACC_MULTIPLYER) {
      local_sum += complexity[j];
      slice_sum++;
      slices[i]++;
      j++;
    }
  }

  // scale slab sizes to input coordinates
  for (int i = 0; i < R; i++) {
    slices[i] = totalSizeY * slices[i]/ (R*RANK_ACC_MULTIPLYER);
  }

	// size of a slab
	const int size_x = totalSizeX;
	const int size_y = slices[rank];
  

	// offsets
	const int offset_x = 0;
	int offset_y = 0;
  for (int i = 0; i < rank; i++) {
    offset_y += slices[i];
  }

	uint8_t* local_image;
  if(rank == 0) {
	  local_image = malloc(NUM_CHANNELS * totalSizeX * totalSizeY * sizeof(uint8_t));
  }
  else {
	  local_image = malloc(NUM_CHANNELS * size_x * size_y * sizeof(uint8_t));
  }

	struct timeval start, end, mid;

	gettimeofday(&start, NULL);

	// comptue the local image
	calcMandelbrot(local_image, offset_x, offset_y, size_x, size_y, totalSizeX, totalSizeY);

	gettimeofday(&mid, NULL);
	double time_before_collect = (mid.tv_sec + mid.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	printf("Rank %d before collect call: %f seconds.\n", rank, time_before_collect);

	// collect all the local images on rank 0
	uint8_t* image = NULL;
	if (rank == 0) {
 		image = malloc(NUM_CHANNELS * totalSizeX * totalSizeY * sizeof(uint8_t) * 4);
	}
  
  for (int i = 0; i < size; ++i) {
      if (rank == i) {
          // Send data to the root process
          if (rank != 0) {
              MPI_Send(local_image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, 0, 0, comm);
          }
      }
  }

  if (rank == 0) {
    for (int i = 1; i < size; i++) {
      int image_offset = 0;
      for (int j = 0; j < i; j++) {
        image_offset += slices[j];
      }
      MPI_Recv(&local_image[image_offset * size_x * NUM_CHANNELS], slices[i] * size_x * NUM_CHANNELS, MPI_UINT8_T, i, 0, comm, MPI_STATUS_IGNORE);
    }
  }
  
	// MPI_Gather(local_image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, 0, comm);


	// measure the exeuction time
	gettimeofday(&end, NULL);
	double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	double maxtimeElapsed = 0;
	MPI_Reduce(&timeElapsed, &maxtimeElapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
	if (rank == 0) {	
		printf("Mandelbrot set calculation for %dx%d took: %f seconds.\n", totalSizeX, totalSizeY, timeElapsed);
	}

	// export the collected image as .png
	if (rank == 0) {
		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi.png", totalSizeX, totalSizeY, NUM_CHANNELS, local_image, stride_bytes);
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