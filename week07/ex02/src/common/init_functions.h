#ifndef INIT_FUNCTIONS
#define INIT_FUNCTIONS

// sample a scalar from a uniform distribution
inline scalar_t runif(scalar_t min, scalar_t max) {
  return min + (rand() / (scalar_t) RAND_MAX) * (max - min);
}

void initialize_positions_Unbalanced(scalar_t* global_masses, vector_t* global_positions, size_t N, int rank) {
  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (rank == ROOT) {
    for (size_t i = 0; i < N / 10; i++) {
      global_masses[i] = MASS;
      global_positions[i].x = runif(0, MAX_COORDINATE);
      global_positions[i].y = runif(0, MAX_COORDINATE);
      global_positions[i].z = runif(0, MAX_COORDINATE);
    }
    for (size_t i = N/10; i < N ; i++) {
      global_masses[i] = MASS;
      global_positions[i].x = runif(0, 10);
      global_positions[i].y = runif(0, 10);
      global_positions[i].z = runif(0, 10);
    }
  }
}

void initialize_positions_Uniform(scalar_t* global_masses, vector_t* global_positions, size_t N, int rank) {
  // initialize the collected positions and masses (only on the root node)
  // otherwise there might be problems with the random numbers
  if (rank == ROOT) {
    for (size_t i = 0; i < N; i++) {
      global_masses[i] = MASS;
      global_positions[i].x = runif(0, MAX_COORDINATE);
      global_positions[i].y = runif(0, MAX_COORDINATE);
      global_positions[i].z = runif(0, MAX_COORDINATE);
    }
  }
}

#endif /* INIT_FUNCTIONS */