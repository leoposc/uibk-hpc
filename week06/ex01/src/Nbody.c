#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_DISTANCE 1
#define G 1
#define FILE_NAME "data.dat"
#define MAX_CORDINATE 100
#define MAX_MASS 10

typedef struct {
  float x;
  float y;
  float z;
} Coordinates;

typedef struct {
  Coordinates positions;
  Coordinates velocity;
  float mass;
} Partciles;

float square(float num) {
  return num * num;
}

float getDistanceSquared(Partciles partcile1, Partciles partcile2) {
  float deltaX = partcile1.positions.x - partcile2.positions.x;
  float deltaY = partcile1.positions.y - partcile2.positions.y;
  float deltaZ = partcile1.positions.z - partcile2.positions.z;

  float distance = square(deltaX) + square(deltaY) + square(deltaZ);
  return (distance > MIN_DISTANCE ? distance : MIN_DISTANCE);
}

void updateUnitVector(Coordinates* unit_vec_address, Partciles partcile1, Partciles partcile2) {
  float deltaX = partcile2.positions.x - partcile1.positions.x;
  float deltaY = partcile2.positions.y - partcile1.positions.y;
  float deltaZ = partcile2.positions.z - partcile1.positions.z;

  float sumOfDelta = sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);

  (*unit_vec_address).x = deltaX / sumOfDelta;
  (*unit_vec_address).y = deltaY / sumOfDelta;
  (*unit_vec_address).z = deltaZ / sumOfDelta;
}

void storePositions(Partciles* partciles, size_t num_particles, FILE* fp) {
  for (size_t i = 0; i < num_particles; i++) {
    fprintf(fp, "%f ", partciles[i].positions.x);
    fprintf(fp, "%f ", partciles[i].positions.y);
    fprintf(fp, "%f\n", partciles[i].positions.z);
  }
  fprintf(fp, "\n");
  fprintf(fp, "\n");
}

int main(int argc, char* argv[]) {
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
    return -1;
  }
#else
  printf("num_particles: %ld\ntime_steps: %ld\n", num_particles, time_steps);
#endif

  clock_t start_time = clock();

  Partciles particles[num_particles];

  for (size_t i = 0; i < num_particles; i++) {
    particles[i].velocity.x = 0;
    particles[i].velocity.y = 0;
    particles[i].velocity.z = 0;

    particles[i].mass = 0.04;

    particles[i].positions.x = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
    particles[i].positions.z = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
    particles[i].positions.y = rand() / (1.0 * RAND_MAX) * MAX_CORDINATE;
  }

  Coordinates unitVector;
  Coordinates force;
  for (size_t t = 0; t < time_steps; t++) {

#ifndef BENCHMARK
    storePositions(particles, num_particles, fp);
#endif

    for (size_t i = 0; i < num_particles; i++) {
      force.x = 0;
      force.y = 0;
      force.z = 0;
      for (size_t j = 0; j < num_particles; j++) {

        if(i == j) {
          continue;
        }

        updateUnitVector(&unitVector, particles[i], particles[j]);
        float distance = getDistanceSquared(particles[i], particles[j]);

        force.x += (G * particles[i].mass * particles[j].mass * unitVector.x) / distance;
        force.y += (G * particles[i].mass * particles[j].mass * unitVector.y) / distance;
        force.z += (G * particles[i].mass * particles[j].mass * unitVector.z) / distance;
      }

      particles[i].velocity.x += force.x / particles[i].mass;
      particles[i].velocity.y += force.y / particles[i].mass;
      particles[i].velocity.z += force.z / particles[i].mass;

      particles[i].positions.x += particles[i].velocity.x;
      particles[i].positions.y += particles[i].velocity.y;
      particles[i].positions.z += particles[i].velocity.z;
    }
  }

  clock_t end_time = clock();
  double elapsed_time = (end_time - start_time) / (double) CLOCKS_PER_SEC;

  printf("$!timesteps_%ld{%ld, %lf}\n", time_steps, num_particles, elapsed_time);

#ifndef BENCHMARK
  fclose(fp);
#endif

  return 0; 
}
