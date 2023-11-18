#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "common/n_body_types.h"
#include "common/init_functions.h"

#define BENCHMARK
// --- global MPI setup ---
const size_t ROOT = 0;
// ------------------------
// --- global simulation constants ---
const scalar_t G = 1.0;
const scalar_t MASS = 1.0;
const scalar_t MAX_COORDINATE = 100.0;
const scalar_t MIN_DISTANCE_SQR = 1.0;
// ----------------------------

// compute the square of a scalar
inline scalar_t square(scalar_t num) {
  return num * num;
}

// store the collected positions to the simulation data
void store_positions(vector_t *positions, size_t num_particles, FILE* fp) {
  for (size_t i = 0; i < num_particles; i++) {
    fprintf(fp, "%f ", positions[i].x);
    fprintf(fp, "%f ", positions[i].y);
    fprintf(fp, "%f\n", positions[i].z);
  }
  fprintf(fp, "\n\n");
}

// create an array of scalars
scalar_t *create_scalars(size_t n) {
  return malloc(n * sizeof(scalar_t));
}

// create an array of 3D vectors
vector_t *create_vects(size_t n) {
  return malloc(n * sizeof(vector_t));
}

// create an array of 3D vectors
particle_t *create_particles(size_t n) {
  return malloc(n * sizeof(particle_t));
}

int main(int argc, char* argv[]) {
  
  // --- MPI setup ---
  MPI_Comm comm;
  int rank;
  int size;
  MPI_Datatype vect_mpi_type;
  MPI_Init(&argc, &argv);
	MPI_Comm_dup(MPI_COMM_WORLD, &comm);
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
  MPI_Type_contiguous(3, MPI_FLOAT, &vect_mpi_type);
  MPI_Type_commit(&vect_mpi_type);
  // -----------------

  // --- parameter setup ---
  size_t num_particles;
  size_t time_steps;
  bool balanced_init;
  if (argc != 4) {
    num_particles = 2;
    time_steps = 20;
    balanced_init = 1;
  } else {
    num_particles = atoi(argv[1]);
    time_steps = atoi(argv[2]);
    balanced_init = atoi(argv[3]);
  }
  // -----------------------


  // --- local MPI setup ---
  const size_t T = time_steps;
  const size_t R = size;
  const size_t N = num_particles;
  const size_t K = N / R;
  const size_t local_offset = K * rank;
  // ------------------------


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

  // --- global simulation setup ---
  scalar_t *global_masses = create_scalars(N);
  vector_t *global_positions = create_vects(N);

  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (balanced_init == 1){
    initialize_positions_Uniform(global_masses, global_positions, N, rank, MAX_COORDINATE, MASS);
  } else {
    initialize_positions_Unbalanced(global_masses, global_positions, N, rank, MAX_COORDINATE, MASS);
  }

  // exchange collected states
  MPI_Bcast(global_masses, N, MPI_FLOAT, ROOT, comm);
  MPI_Bcast(global_positions, N, vect_mpi_type, ROOT, comm);
  // ------------------------------

  // --- local simulation setup ---
  particle_t *local_particles = create_particles(K);
  for (size_t k = 0; k < K; k++) {
    local_particles[k].mass = global_masses[local_offset + k];
    local_particles[k].p.x = global_positions[local_offset + k].x;
    local_particles[k].p.y = global_positions[local_offset + k].y;
    local_particles[k].p.z = global_positions[local_offset + k].z;
    local_particles[k].v.x = 0.0;
    local_particles[k].v.y = 0.0;
    local_particles[k].v.z = 0.0;
  }
  // ------------------------------

  vector_t *local_position_buffer = create_vects(K);

  // start measuring the time
  clock_t start_time = clock();

  // for every time step...
  for (size_t t = 0; t < T; t++) {

#ifndef BENCHMARK   
    // write the current state to the disc 
    if (rank == ROOT) {
      store_positions(global_positions, N, fp);
    }
#endif

    // for every local particle...
    for (size_t k = 0; k < K; k++) {

      // initialize the forces
      vector_t f = {
        0,
        0,
        0
      };

      // compute the corresponding index for i in the collected particles
      size_t i = local_offset + k;

      // for every state of the previous particles...
      for (size_t j = 0; j < N; j++) {

        // do not compute interaction with itself
        if(i == j) {
          continue;
        }

        // load the respective masses
        scalar_t m_i = global_masses[i];
        scalar_t m_j = global_masses[j];

        // load the respective positions
        vector_t p_i = global_positions[i];
        vector_t p_j = global_positions[j];

        // compute the displacement between two particles
        vector_t d = {
          p_j.x - p_i.x,
          p_j.y - p_i.y,
          p_j.z - p_i.z
        };

        // compute the squared distance
        scalar_t distance_sqr = square(d.x) + square(d.y) + square(d.z);

        // avoid division by 0 and reduce the effects of floating point errors
        if (distance_sqr < MIN_DISTANCE_SQR) {
          distance_sqr = MIN_DISTANCE_SQR;
        }

        // compute the magnitude of the force
        scalar_t f_mag = G * m_i * m_j / distance_sqr;

        // compute the regular distance
        scalar_t distance = sqrt(distance_sqr);

        // update the forces using the equation of gravity
        f.x += f_mag * d.x / distance;
        f.y += f_mag * d.y / distance;
        f.z += f_mag * d.z / distance;
      }

      // update the local velocities accordingly
      local_particles[k].v.x += f.x / local_particles[k].mass;
      local_particles[k].v.y += f.y / local_particles[k].mass;
      local_particles[k].v.z += f.z / local_particles[k].mass;

      // update the local positions accordingly
      local_particles[k].p.x += local_particles[k].v.x;
      local_particles[k].p.y += local_particles[k].v.y;
      local_particles[k].p.z += local_particles[k].v.z;

      // set the element in the local position buffer
      local_position_buffer[k].x = local_particles[k].p.x;
      local_position_buffer[k].y = local_particles[k].p.y;
      local_position_buffer[k].z = local_particles[k].p.z;
    }

    // --- MPI EXCHANGE ---

    MPI_Request send_reqs[R];
    MPI_Request recv_reqs[R];

    // send local updated positions to every other rank
    for (size_t r = 0; r < R; r++) {
      MPI_Isend(local_position_buffer, K, vect_mpi_type, r, 0, comm, &send_reqs[r]);
    }

    // receive updated positions from every other rank
    for (size_t r = 0; r < R; r++) {
      MPI_Irecv(&global_positions[K * r], K, vect_mpi_type, r, 0, comm, &recv_reqs[r]);
    }

    MPI_Waitall(R, recv_reqs, MPI_STATUSES_IGNORE);
    MPI_Waitall(R, send_reqs, MPI_STATUSES_IGNORE);

    // ---------------------------
  }

  // print the benchmark information
  if (rank == ROOT) {
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("$!%s_%ld_timesteps_%ld_size{%d, %lf}\n", balanced_init ? "balanced" : "unbalanced", time_steps, num_particles, size, elapsed_time);
  }

  // clean up
  free(local_particles);
  free(local_position_buffer);
  free(global_masses);
  free(global_positions);
  MPI_Type_free(&vect_mpi_type);

#ifndef BENCHMARK
  if (rank == ROOT) {
    fclose(fp);
  }
#endif

  MPI_Finalize();

  return EXIT_SUCCESS; 
}
