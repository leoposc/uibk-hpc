// Struct of Arrays implementation
#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//#define BENCHMARK

typedef float scalar_t;
typedef scalar_t *Vector;

typedef struct {
  Vector masses;
  Vector p_xs;
  Vector p_ys;
  Vector p_zs;
  Vector v_xs;
  Vector v_ys;
  Vector v_zs;
} particles;

// create an array of scalars
scalar_t *create_scalars(size_t n) {
  return malloc(n * sizeof(scalar_t));
}

inline scalar_t runif(scalar_t min, scalar_t max) {
  return min + (rand() / (scalar_t) RAND_MAX) * (max - min);
}

// compute the square of a scalar
inline scalar_t square(scalar_t num) {
  return num * num;
}

void customfree(particles p){
  free(p.masses);
  free(p.p_xs);
  free(p.p_ys);
  free(p.p_zs);
  free(p.v_xs);
  free(p.v_ys);
  free(p.v_zs);
}


// store the collected positions to the simulation data
void store_positions(particles p, size_t num_particles, FILE* fp) {
  for (size_t i = 0; i < num_particles; i++) {
    fprintf(fp, "%f ", p.p_xs[i]);
    fprintf(fp, "%f ", p.p_ys[i]);
    fprintf(fp, "%f\n", p.p_zs[i]);
  }
  fprintf(fp, "\n\n");
}

int main(int argc, char* argv[]) {
	
  // --- MPI setup ---
  //MPI_Comm comm;
  int rank;
  int size;
  MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
  // -----------------

  // --- parameter setup ---
  size_t num_particles;
  size_t time_steps;
  if (argc != 3) {
    num_particles = 2;
    time_steps = 20;
  } else {
    num_particles = atoi(argv[1]);
    time_steps = atoi(argv[2]);
  }
  // -----------------------

  // --- global MPI setup ---
  const size_t ROOT = 0;
  // ------------------------

  // --- local MPI setup ---
  const size_t T = time_steps;
  const size_t R = size;
  const size_t N = num_particles;
  const size_t K = N / R;
  const size_t local_offset = K * rank;
  // ------------------------

  // --- global simulation constants ---
  const scalar_t G = 1.0;
  const scalar_t MASS = 1.0;
  const scalar_t MAX_COORDINATE = 100.0;
  const scalar_t MIN_DISTANCE_SQR = 1.0;
  // ----------------------------

#ifndef BENCHMARK
  const char *OUT_FILE = "data.dat";
  FILE *fp = NULL;
  if (rank == ROOT) {
    fp = fopen(OUT_FILE, "w");
    if (fp == NULL) {
      printf("file opening failed\n");
      MPI_Finalize();
      return EXIT_FAILURE;
    }
  }
#endif

  
  particles globalParticles;
  globalParticles.p_xs = create_scalars(N);
  globalParticles.p_ys = create_scalars(N);
  globalParticles.p_zs = create_scalars(N);
  globalParticles.v_xs = create_scalars(N);
  globalParticles.v_ys = create_scalars(N);
  globalParticles.v_zs = create_scalars(N);
  globalParticles.masses = create_scalars(N);
  

  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (rank == ROOT) {
    // --- global simulation setup ---
    for (size_t i = 0; i < N; i++) {
      globalParticles.masses[i] = MASS;
      globalParticles.p_xs[i] = runif(0, MAX_COORDINATE);
      globalParticles.p_ys[i] = runif(0, MAX_COORDINATE);
      globalParticles.p_zs[i] = runif(0, MAX_COORDINATE);
    }
  }

  // local particles 
  particles localParticles;
  localParticles.p_xs = create_scalars(K);
  localParticles.p_ys = create_scalars(K);
  localParticles.p_zs = create_scalars(K);
  localParticles.v_xs = create_scalars(K);
  localParticles.v_ys = create_scalars(K);
  localParticles.v_zs = create_scalars(K);
  localParticles.masses = create_scalars(K);

  for (size_t i = 0; i < K; i++) {
      localParticles.masses[i] = MASS;
      localParticles.v_xs[i] = 0.0;
      localParticles.v_ys[i] = 0.0;
      localParticles.v_zs[i] = 0.0;
    }

  MPI_Bcast(globalParticles.p_xs, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(globalParticles.p_ys, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  MPI_Bcast(globalParticles.p_zs, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);

  // start measuring the time
  clock_t start_time = clock();

  // for every time step...
  for (size_t t = 0; t < T; t++) {

#ifndef BENCHMARK   
    // write the current state to the disc 
    if (rank == ROOT) {
      store_positions(globalParticles, N, fp);
    }
#endif
    // for every local particle...
    for (size_t k = 0; k < K; k++) {


      scalar_t f_x = 0.0;
      scalar_t f_y = 0.0;
      scalar_t f_z = 0.0;

      // compute the corresponding index for i in the collected particles
      size_t i = local_offset + k;

      // for every state of the previous particles...
      for (size_t j = 0; j < N; j++) {

        // do not compute interaction with itself
        if(i == j) {
          continue;
        }

        // load the respective masses
        scalar_t m_i = globalParticles.masses[i];
        scalar_t m_j = globalParticles.masses[j];

        // compute the squared distance
        scalar_t d_x = globalParticles.p_xs[j] - globalParticles.p_xs[i];
        scalar_t d_y = globalParticles.p_ys[j] - globalParticles.p_ys[i];
        scalar_t d_z = globalParticles.p_zs[j] - globalParticles.p_zs[i];

        scalar_t distance_sqr = square(d_x) + square(d_y) + square(d_z);

        // avoid division by 0 and reduce the effects of floating point errors
        if (distance_sqr < MIN_DISTANCE_SQR) {
          distance_sqr = MIN_DISTANCE_SQR;
        }

        // compute the magnitude of the force
        scalar_t f_mag = G * m_i * m_j / distance_sqr;

        // compute the regular distance
        scalar_t distance = sqrt(distance_sqr);

        // update the forces using the equation of gravity
        f_x += f_mag * d_x / distance;
        f_y += f_mag * d_y / distance;
        f_z += f_mag * d_z / distance;
      }

      // update the local velocities accordingly
      localParticles.v_xs[k] += f_x / localParticles.masses[k];
      localParticles.v_ys[k] += f_y / localParticles.masses[k];
      localParticles.v_zs[k] += f_z / localParticles.masses[k];

      // update the local positions accordingly
      localParticles.p_xs[k] += localParticles.v_xs[k];
      localParticles.p_ys[k] += localParticles.v_ys[k];
      localParticles.p_zs[k] += localParticles.v_zs[k];

    }

    // gather the local results from all ranks
    MPI_Gather(localParticles.p_xs, K, MPI_FLOAT, globalParticles.p_xs, K, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Gather(localParticles.p_ys, K, MPI_FLOAT, globalParticles.p_ys, K, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Gather(localParticles.p_zs, K, MPI_FLOAT, globalParticles.p_zs, K, MPI_FLOAT, ROOT, MPI_COMM_WORLD);

    // broadcast the updated collected particles back to the ranks
    MPI_Bcast(globalParticles.p_xs, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(globalParticles.p_ys, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(globalParticles.p_zs, N, MPI_FLOAT, ROOT, MPI_COMM_WORLD);
  }

  // print the benchmark information
  if (rank == ROOT) {
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("$!timesteps_%ld{%d, %ld, %lf}\n", time_steps, size, num_particles, elapsed_time);
  }

  // clean up
  customfree(localParticles);
  customfree(globalParticles);

#ifndef BENCHMARK
  if (rank == ROOT) {
    fclose(fp);
  }
#endif

  MPI_Finalize();

  return EXIT_SUCCESS; 
}
