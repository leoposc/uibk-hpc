#ifndef N_BODY_TYPES
#define N_BODY_TYPES

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

#endif /* INIT_FUNCTIONS */