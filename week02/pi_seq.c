#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    double starttime = MPI_Wtime();
    if(argc != 2) {
        printf("Usage: %s <sample_size>\n", argv[0]);
        return 1;
    }
    int sample_size = atoi(argv[1]);    
    printf("Computed %f for Pi\n", monte_carlo_pi(sample_size));
    double endtime = MPI_Wtime();
    printf("Time ellapsed: %f\n", endtime - starttime);
    return 0;
}