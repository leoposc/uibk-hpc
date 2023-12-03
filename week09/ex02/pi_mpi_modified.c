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
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);
    MPI_Comm_size(comm, &size);
    MPI_Comm_rank(comm, &rank);
    MPI_Request reqs;

    double starttime, endtime;
    float pi = 0.0;
    float result = 0.0;
    int ready = 0;
    starttime = MPI_Wtime();
    
    if (rank == 0) {
        
        if(argc != 2) {
            printf("Usage: %s <sample_size>\n", argv[0]);
            return 1;
        }
        int sample_size = atoi(argv[1]);
        // compute the pi value
        pi = monte_carlo_pi(sample_size);
    } else  {
        printf("Process %d is running\n", rank);
    }

    MPI_Barrier(comm);

    // MPI_Ireduce(&pi, &result, 1, MPI_FLOAT, MPI_SUM, 0, comm, &reqs);

    // // while (ready == 0) {
    //     MPI_Test(&reqs, &ready, MPI_STATUS_IGNORE);
    //     nanosleep((const struct timespec[]){{0, 1000000000000}}, NULL);
    // }

    

    if (rank == 0) {
        endtime = MPI_Wtime();
        printf("Parallel programm finished. %d processes ran in total.\n", size);
        printf("Computed %f for Pi\n", result);
        printf("Time ellapsed: %f\n", endtime - starttime);
    }

    MPI_Finalize();
    return 0;
}