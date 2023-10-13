#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

float monte_carlo_pi(unsigned int sample_size) {
    double count = 0;
    srand(time(NULL));
    for(unsigned int i = 0; i < sample_size; i++) {
        float x = (float)rand() / RAND_MAX;
        float y = (float)rand() / RAND_MAX;
        if(x * x + y * y <= 1) {
            count++;
        }
    }
    return 4 * (count / sample_size);
}

int main(int argc, char** argv) {
    /* run the monte carlo pi algorithm with the given sample size
       as a parallel MPI program */
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(argc != 2) {
        printf("Usage: %s <sample_size>\n", argv[0]);
        return 1;
    }
    int sample_size = atoi(argv[1]);
    int local_sample_size = floor(sample_size / size);    
    // compute the local pi value for each process
    float local_pi = monte_carlo_pi(local_sample_size);
    // compute the global pi value
    float global_pi;
    MPI_Reduce(&local_pi, &global_pi, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if(rank == 0) {
        global_pi /= size;
        printf("Parallel programm finished. %d processes ran in total.\n", size);
        printf("Computed %f for Pi\n", global_pi);
    }

    MPI_Finalize();
    return 0;
}