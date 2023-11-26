#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef float scalar_t;

typedef struct {
  scalar_t x;
  scalar_t y;
  scalar_t z;
} vector_t;

typedef struct {
  scalar_t mass;
  vector_t p;
  vector_t v;
} particle_t;

// sample a scalar from a uniform distribution
inline scalar_t runif(scalar_t min, scalar_t max) {
  return min + (rand() / (scalar_t) RAND_MAX) * (max - min);
}

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
  int do_benchmark;
  if (argc != 4) {
    num_particles = 2;
    time_steps = 20;
    do_benchmark = 0;
  } else {
    num_particles = atoi(argv[1]);
    time_steps = atoi(argv[2]);
    do_benchmark = atoi(argv[3]);
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
  const char *OUT_FILE = "data.dat";
  // ----------------------------

  FILE *fp = NULL;
  if (!do_benchmark && rank == ROOT) {
    fp = fopen(OUT_FILE, "w");
    if (fp == NULL) {
      printf("file opening failed\n");
      MPI_Finalize();
      return EXIT_FAILURE;
    }
  }

  // --- local simulation setup ---

  // initialize RNG based on the rank of the process
  srand(rank);

  // allocate local memory
  particle_t *local_particles = create_particles(K);
  scalar_t *global_masses = create_scalars(N);
  vector_t *global_positions = create_vects(N);

  // initialize the local particles
  for (size_t i = 0; i < K; i++) {
    local_particles[i].mass = MASS;
    local_particles[i].p.x = runif(0, MAX_COORDINATE);
    local_particles[i].p.y = runif(0, MAX_COORDINATE);
    local_particles[i].p.z = runif(0, MAX_COORDINATE);
    local_particles[i].v.x = 0.0;
    local_particles[i].v.y = 0.0;
    local_particles[i].v.z = 0.0;
  }
  // -----------------------------

  // --- window setup ---

  // create window to expose the masses of local porticles
  MPI_Win mass_win;
  scalar_t *local_mass_buf;
  MPI_Win_allocate(K * sizeof(scalar_t), sizeof(scalar_t), MPI_INFO_NULL,
      comm, &local_mass_buf, &mass_win);

  // create window to expose the positions of local porticles
  MPI_Win pos_win;
  vector_t *local_pos_buf;
  MPI_Win_allocate(K * sizeof(vector_t), sizeof(vector_t), MPI_INFO_NULL,
      comm, &local_pos_buf, &pos_win);

  // ---------------------

  // --- initial mass exchange ---

  // update the current masses in the local window
  for (size_t i = 0; i < K; i++) {
    local_mass_buf[i] = local_particles[i].mass;
  }

  // fetch the masses from the other processes
  MPI_Win_fence(0, mass_win);
  for (size_t r = 0; r < R; r++) {
    MPI_Get(global_masses + r*K, K, MPI_FLOAT, r, 0, K, MPI_FLOAT, mass_win);
  }
  MPI_Win_fence(0, mass_win);

  // ------------------------------------------

  // --- initial position exchange ---

  // update the current positions in the local window
  for (size_t i = 0; i < K; i++) {
    local_pos_buf[i].x = local_particles[i].p.x;
    local_pos_buf[i].y = local_particles[i].p.y;
    local_pos_buf[i].z = local_particles[i].p.z;
  }

  // fetch the positions from the other processes
  MPI_Win_fence(0, pos_win);
  for (size_t r = 0; r < R; r++) {
    MPI_Get(global_positions + r*K, K, vect_mpi_type, r, 0, K, vect_mpi_type, pos_win);
  }
  MPI_Win_fence(0, pos_win);

  // ------------------------------------------

  // start measuring the time
  clock_t start_time = clock();

  // for every time step...
  for (size_t t = 0; t < T; t++) {

    // write the current state to the disc 
    if (!do_benchmark && rank == ROOT) {
      store_positions(global_positions, N, fp);
    }

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

      // update the positions in the local window
      local_pos_buf[k].x = local_particles[k].p.x;
      local_pos_buf[k].y = local_particles[k].p.y;
      local_pos_buf[k].z = local_particles[k].p.z;
    }

    // fetch the positions from the other processes
    MPI_Win_fence(MPI_MODE_NOPRECEDE | MPI_MODE_NOSTORE, pos_win);
    for (size_t r = 0; r < R; r++) {
      MPI_Get(global_positions + r*K, K, vect_mpi_type, r, 0, K, vect_mpi_type, pos_win);
    }
    MPI_Win_fence(MPI_MODE_NOSUCCEED | MPI_MODE_NOPUT, pos_win);
  }

  // print the benchmark information
  if (rank == ROOT) {
    clock_t end_time = clock();
    double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;
    printf("$!%ld_timesteps_%ld_size{%d, %lf, %d}\n", time_steps, num_particles, size, elapsed_time, do_benchmark);
  }

  // clean up
  free(local_particles);
  MPI_Win_free(&mass_win);
  MPI_Win_free(&pos_win);
  MPI_Type_free(&vect_mpi_type);

  if (!do_benchmark && rank == ROOT) {
    fclose(fp);
  }

  MPI_Finalize();

  return EXIT_SUCCESS; 
}
