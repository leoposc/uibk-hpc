#include "n_body_types.h"

#ifndef INIT_FUNCTIONS
#define INIT_FUNCTIONS

// sample a scalar from a uniform distribution
inline scalar_t runif(scalar_t min, scalar_t max) {
  return min + (rand() / (scalar_t) RAND_MAX) * (max - min);
}

void initialize_positions_Unbalanced(scalar_t* global_masses, vector_t* global_positions, size_t N, int rank, scalar_t max_cord, scalar_t mass) {
  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (rank == 0) {
    for (size_t i = 0; i < N / 10; i++) {
      global_masses[i] = mass;
      global_positions[i].x = runif(0, max_cord);
      global_positions[i].y = runif(0, max_cord);
      global_positions[i].z = runif(0, max_cord);
    }
    for (size_t i = N/10; i < N ; i++) {
      global_masses[i] = mass;
      global_positions[i].x = runif(0, 10);
      global_positions[i].y = runif(0, 10);
      global_positions[i].z = runif(0, 10);
    }
  }
}

void initialize_positions_Uniform(scalar_t* global_masses, vector_t* global_positions, size_t N, int rank, scalar_t max_cord, scalar_t mass) {
  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (rank == 0) {
    for (size_t i = 0; i < N; i++) {
      global_masses[i] = mass;
      global_positions[i].x = runif(0, max_cord);
      global_positions[i].y = runif(0, max_cord);
      global_positions[i].z = runif(0, max_cord);
    }
  }
}

#endif /* INIT_FUNCTIONS */