#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MIN_DISTANCE 1.0
#define G 1.0
#define FILE_NAME "data.dat"
#define MAX_CORDINATE 100.0
#define MAX_MASS 10.0

typedef struct {
  float mass;
  float px;
  float py;
  float pz;
  float vx;
  float vy;
  float vz;
} particle_t;

inline float square(float num) {
  return num * num;
}

void storePositions(particle_t *partciles, size_t num_particles, FILE* fp) {
  for (size_t i = 0; i < num_particles; i++) {
    fprintf(fp, "%f ", partciles[i].px);
    fprintf(fp, "%f ", partciles[i].py);
    fprintf(fp, "%f\n", partciles[i].pz);
  }
  fprintf(fp, "\n\n");
}

// create a 1D array of particles
particle_t *create_particles(size_t n) {
  return malloc(n * sizeof(particle_t));
}

int main(int argc, char* argv[]) {
	
  MPI_Init(&argc, &argv);

	MPI_Comm comm;
	MPI_Comm_dup(MPI_COMM_WORLD, &comm);

	int rank;
	MPI_Comm_rank(comm, &rank);

  int size;
	MPI_Comm_size(comm, &size);

  size_t num_particles;
  size_t time_steps;
  if (argc != 3) {
    num_particles = 2;
    time_steps = 20;
  }
  else {
    num_particles = atoi(argv[1]);
    time_steps = atoi(argv[2]);
  }

#ifndef BENCHMARK
  FILE *fp = fopen(FILE_NAME, "w");
  if (fp == NULL) {
    printf("file opening failed\n");
    return EXIT_FAILURE;
  }
#else
  printf("num_particles: %ld\ntime_steps: %ld\n", num_particles, time_steps);
#endif

  // define the particle_t struct type
  MPI_Datatype particle_type;
  int blockLengths[7] = {1, 1, 1, 1, 1, 1, 1};
  MPI_Aint displacements[7];
  MPI_Datatype types[7] = {MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT, MPI_FLOAT};

  // calculate the displacements of each struct member
  MPI_Get_address(&(((particle_t *)0)->mass), &displacements[0]);
  MPI_Get_address(&(((particle_t *)0)->px), &displacements[1]);
  MPI_Get_address(&(((particle_t *)0)->py), &displacements[2]);
  MPI_Get_address(&(((particle_t *)0)->pz), &displacements[3]);
  MPI_Get_address(&(((particle_t *)0)->vx), &displacements[4]);
  MPI_Get_address(&(((particle_t *)0)->vy), &displacements[5]);
  MPI_Get_address(&(((particle_t *)0)->vz), &displacements[6]);

  // adjust the displacements to be relative to the struct address
  for (int i = 6; i > 0; i--) {
      displacements[i] -= displacements[0];
  }
  displacements[0] = 0;

  MPI_Type_create_struct(7, blockLengths, displacements, types, &particle_type);
  MPI_Type_commit(&particle_type);

  const size_t ROOT = 0;
  const size_t T = time_steps;
  const size_t R = size;
  const size_t N = num_particles;
  const size_t K = N / R;

  const size_t offset = K*rank;

  if (rank == ROOT) {
    printf("N: %ld\n", N);  
    printf("T: %ld\n", T);
    printf("R: %ld\n", R);
    printf("K: %ld\n", K);
  }

  // allocate memory for the collected particles on each rank
  particle_t *collected_particles = create_particles(N);

  // allocate memory for the local particles on each rank
  particle_t *local_particles = create_particles(K);

  // initialize the collected particles only on the root node
  // otherwise there might be problems with the random numbers
  if (rank == ROOT) {
    for (size_t i = 0; i < N; i++) {
      collected_particles[i].mass = 1.0;
      collected_particles[i].vx = 0.0;
      collected_particles[i].vy = 0.0;
      collected_particles[i].vz = 0.0;
      collected_particles[i].px = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
      collected_particles[i].py = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
      collected_particles[i].pz = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
    }
  }

  // broadcast the initial state to the other ranks
  MPI_Bcast(collected_particles, N, particle_type, ROOT, comm);

  // on each rank, initialize the local particles
  memcpy(local_particles, collected_particles + K * rank, K * sizeof(particle_t));

  // start measuring the time
  clock_t start_time = clock();

  // for every time step...
  for (size_t t = 0; t < T; t++) {

#ifndef BENCHMARK   
    // write the current state to the disc 
    if (rank == ROOT) {
      storePositions(collected_particles, N, fp);
    }
#endif

    // for every local particle...
    for (size_t k = 0; k < K; k++) {

      // initialize the forces
      float fx = 0;
      float fy = 0;
      float fz = 0;

      // compute the corresponding index for i in the collected particles
      size_t i = offset + k;

      // for every state of the previous particles...
      for (size_t j = 0; j < N; j++) {

        // do not compute interaction with itself
        if(i == j) {
          continue;
        }

        // compute the displacement between two particles
        float dx = collected_particles[j].px - collected_particles[i].px;
        float dy = collected_particles[j].py - collected_particles[i].py;
        float dz = collected_particles[j].pz - collected_particles[i].pz;

        // compute the squared distance
        float distance_sqr = square(dx) + square(dy) + square(dz);

        // avoid division by 0 and reduce the effects of floating point errors
        if (distance_sqr < square(MIN_DISTANCE)) {
          distance_sqr = square(MIN_DISTANCE);
        }

        // compute the regular distance
        float distance = sqrt(distance_sqr);

        // compute the magnitude of the force
        float f_mag = G * collected_particles[i].mass * collected_particles[j].mass / distance_sqr;

        // compute the unit verctor for the displacement
        float ux = dx / distance;
        float uy = dy / distance;
        float uz = dz / distance;

        // update the forces using the equation of gravity
        fx += f_mag * ux;
        fy += f_mag * uy;
        fz += f_mag * uz;
      }

      // update the velocities accordingly
      local_particles[k].vx += fx / local_particles[k].mass;
      local_particles[k].vy += fy / local_particles[k].mass;
      local_particles[k].vz += fz / local_particles[k].mass;

      // update the positions accordingly
      local_particles[k].px += local_particles[k].vx;
      local_particles[k].py += local_particles[k].vy;
      local_particles[k].pz += local_particles[k].vz;
    }

    // gather the local results from all ranks
    MPI_Gather(local_particles, K, particle_type, collected_particles, K, particle_type, ROOT, comm);

    // broadcast the updated collected particles back to the ranks
    MPI_Bcast(collected_particles, N, particle_type, ROOT, comm);
  }

  // print the benchmark information
  if (rank == ROOT) {
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;

    printf("$!timesteps_%ld{%d, %ld, %lf}\n", time_steps, size, num_particles, elapsed_time);
  }

  // clean up
  free(local_particles);
  free(collected_particles);
  MPI_Type_free(&particle_type);

#ifndef BENCHMARK
  fclose(fp);
#endif

  MPI_Finalize();

  return EXIT_SUCCESS; 
}
