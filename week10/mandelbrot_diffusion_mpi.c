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

void updateGridSize(double *uniformGrid, double *diffusedGrid, int grid_size) {
	double diff_param = 0.001;
	double val;
    // move particles towards higher concentration
    for (int i = 1; i < grid_size - 1; i++) {
		val = (diffusedGrid[i - 1] + diffusedGrid[i + 1] - 2*diffusedGrid[i]);
        uniformGrid[i] += fabs(diff_param/val) * val;
    }
	val = (diffusedGrid[grid_size - 1] + diffusedGrid[1] - 2*diffusedGrid[0]);
	uniformGrid[0] += fabs(diff_param/val) *val;
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
	

	// 1. Sub sampling
	int subTotalSizeX = totalSizeX / 10;
	int subTotalSizeY = totalSizeY / 10;
	int size_x = subTotalSizeX;
	int size_y = subTotalSizeY / R;
	int offset_y = rank * size_y;
	uint8_t* subsampled_local_image = malloc(NUM_CHANNELS* size_x * size_y * sizeof(uint8_t));

	struct timeval start_sample, end_sample;

	gettimeofday(&start_sample, NULL);

	calcMandelbrot(subsampled_local_image, 0, offset_y, size_x, size_y, subTotalSizeX, subTotalSizeY);
	gettimeofday(&end_sample, NULL);
	free(subsampled_local_image);
	double un_balanced_time = (end_sample.tv_sec + end_sample.tv_usec * 1e-6) - (start_sample.tv_sec + start_sample.tv_usec * 1e-6);

	double *unbalanced_time_collection = NULL;
	if (rank == 0){
		unbalanced_time_collection = malloc(sizeof(double)* size);
		
	}

	MPI_Gather(&un_balanced_time, 1, MPI_DOUBLE, unbalanced_time_collection, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	
	// 2. Balanced computation

	// workload diffusion
	int local_offset_y, local_size_y;
	int *offsets_y = malloc(sizeof(int)*size);
	int *sizes_y = malloc(sizeof(int)*size);
	if (rank == 0){
		
		double *temp = malloc(sizeof(double)*size);
		double *post_diff = malloc(sizeof(double)*size);
		double *y_points_per_rank = malloc(sizeof(double)*size);
		for (int i = 0; i < size;i++){
			y_points_per_rank[i] = (totalSizeY / (double)size);
			post_diff[i] = unbalanced_time_collection[i];
		}

		for (int t = 0; t < 5000; t++){ //1000
			for (int i = 0; i < size; i++){
				double tc = post_diff[i];

      			// get time of adjacent cells
      			double tl = (i != 0) ? post_diff[i - 1] : post_diff[size - 1];
      			double tr = (i != size - 1) ? post_diff[i + 1] : post_diff[0];

      			// compute new time at current position
      			temp[i] = tc + 0.2 * (tl + tr + (-2 * tc));
			}

			for (int i = 0; i < size; i++) {
            	post_diff[i] = temp[i];
        	}
			updateGridSize(y_points_per_rank, post_diff, size);
		}
		free(temp);

		local_offset_y = 0;
		local_size_y = floor(y_points_per_rank[0]);

		int total_points_to_split = totalSizeY - local_size_y;
		int send_local_size_y;
		int send_local_offset_y = local_size_y;
		offsets_y[0] = local_offset_y;
		sizes_y[0] = local_size_y;
		// printf("Rank 0: offset: %d, size: %d\n", local_offset_y, local_size_y);
		for (int i = 1; i < size; i++){
			if (i < size - 1){
				total_points_to_split = total_points_to_split - floor(y_points_per_rank[i]);
				
				send_local_size_y = floor(y_points_per_rank[i]);
				offsets_y[i] = send_local_offset_y;
				sizes_y[i] = send_local_size_y;
				// printf("Rank %d: offset: %d, size: %d\n",i, send_local_offset_y, send_local_size_y);
				MPI_Send(&send_local_size_y, 1, MPI_INT, i, i, comm);
				MPI_Send(&send_local_offset_y, 1, MPI_INT, i, i*2, comm);
				send_local_offset_y = send_local_offset_y + send_local_size_y;
			} 
			else{
				send_local_size_y = total_points_to_split;
				MPI_Send(&send_local_size_y, 1, MPI_INT, i, i, comm);
				MPI_Send(&send_local_offset_y, 1, MPI_INT, i, i*2, comm);
				offsets_y[size-1] = send_local_offset_y;
				sizes_y[i] = send_local_size_y;
				// printf("Rank %d: offset: %d, size: %d\n",i, send_local_offset_y, send_local_size_y);
			}
		}
	}
	else {
		MPI_Recv(&local_size_y, 1, MPI_INT, 0, rank, comm, MPI_STATUS_IGNORE);
		MPI_Recv(&local_offset_y, 1, MPI_INT, 0, rank*2, comm, MPI_STATUS_IGNORE);
	}


	//2. Balanced computation

	// size of a slab
	size_x = totalSizeX;
	size_y = local_size_y;

	// offsets
	const int offset_x = 0;
	offset_y = local_offset_y;

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
	printf("%d %f\n", rank, time_before_collect);

	// collect all the local images on rank 0
  	if (rank != 0) {
  	    // Send data to the root process
  	    MPI_Send(local_image, size_x * size_y * NUM_CHANNELS, MPI_UINT8_T, 0, 0, comm);
  	}


  	if (rank == 0) {
  	  for (int i = 1; i < size; i++) {
  	    MPI_Recv(&local_image[offsets_y[i] * size_x * NUM_CHANNELS], sizes_y[i] * size_x * NUM_CHANNELS, MPI_UINT8_T, i, 0, comm, MPI_STATUS_IGNORE);
  	  }
  	}

	// measure the exeuction time
	gettimeofday(&end, NULL);
	double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	double maxtimeElapsed = 0;
	MPI_Reduce(&timeElapsed, &maxtimeElapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);
	if (rank == 0) {	
		printf("Mandelbrot set calculation for %dx%d took: %f seconds.\n", totalSizeX, totalSizeY, maxtimeElapsed);
	}

	// export the collected image as .png
	if (rank == 0) {
		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi_diffusion.png", totalSizeX, totalSizeY, NUM_CHANNELS, local_image, stride_bytes);
	}

	free(local_image);
	
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