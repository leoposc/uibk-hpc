#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN_DISTANCE 0.1
#define G 1
#define FILE_NAME "data.dat"
#define MAX_CORDINATE 100
#define MAX_MASS 10

typedef struct {
  double x;
  double y;
  double z;
} Coordinates;

typedef struct {
  Coordinates positions;
  Coordinates velocity;
  Coordinates force;
  double mass;
} Partciles;

double square(double num) {
  return num * num;
}

double getDistanceSquared(Partciles partcile1, Partciles partcile2) {
  double deltaX = partcile1.positions.x - partcile2.positions.x;
  double deltaY = partcile1.positions.y - partcile2.positions.y;
  double deltaZ = partcile1.positions.z - partcile2.positions.z;

  return sqrt(square(deltaX) + square(deltaY) + square(deltaZ));
}

void updateUnitVector(Coordinates* unit_vec_address, Partciles partcile1, Partciles partcile2) {
  double deltaX = partcile2.positions.x - partcile1.positions.x;
  double deltaY = partcile2.positions.y - partcile1.positions.y;
  double deltaZ = partcile2.positions.z - partcile1.positions.z;

  double sumOfDelta = sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);

  (*unit_vec_address).x = deltaX / sumOfDelta;
  (*unit_vec_address).y = deltaY / sumOfDelta;
  (*unit_vec_address).z = deltaZ / sumOfDelta;
}

void storePositions(Partciles* partciles, size_t num_particles, FILE* fp) {
  for (size_t i = 0; i < num_particles; i++) {
    fprintf(fp, "%f ", partciles[i].positions.x);
    fprintf(fp, "%f ", partciles[i].positions.y);
    fprintf(fp, "%f\n", partciles[i].positions.y);
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


  FILE *fp = fopen(FILE_NAME, "w");
  if (fp == NULL) {
    printf("file opening failed\n");
    return -1;
  }

  Partciles particles[num_particles];

  for (size_t i = 0; i < num_particles; i++) {
    particles[i].velocity.x = 0;
    particles[i].velocity.y = 0;
    particles[i].velocity.z = 0;

    particles[i].force.x = 0;
    particles[i].force.y = 0;
    particles[i].force.z = 0;

    particles[i].mass = 1;

    particles[i].positions.x = rand() % MAX_CORDINATE;
    particles[i].positions.y = rand() % MAX_CORDINATE;
    particles[i].positions.z = rand() % MAX_CORDINATE;
  }


  Coordinates unitVector;
  for (size_t t = 0; t < time_steps; t++) {
    storePositions(particles, num_particles, fp);
    for (size_t i = 0; i < num_particles; i++) {
      particles[i].force.x = 0;
      particles[i].force.y = 0;
      particles[i].force.z = 0;
      for (size_t j = 0; j < num_particles; j++) {
        //adjust force of particle i impacted by all other particles
        if(i == j) {
          // skip itslef, or particle on its place
          continue;
        }
        //avoid dividing by 0
        // fix direciton with last multiplyer
        updateUnitVector(&unitVector, particles[i], particles[j]);
        double distance = getDistanceSquared(particles[i], particles[j]);

        particles[i].force.x += (G * particles[i].mass * particles[j].mass * unitVector.x) / distance;

        particles[i].force.y += (G * particles[i].mass * particles[j].mass * unitVector.y) / distance;

        particles[i].force.z += (G * particles[i].mass * particles[j].mass * unitVector.z) / distance;
      }
      particles[i].force.x /= num_particles;
      particles[i].force.y /= num_particles;
      particles[i].force.z /= num_particles;
      particles[i].velocity.x += particles[i].force.x / particles[i].mass;
      particles[i].velocity.y += particles[i].force.y / particles[i].mass;
      particles[i].velocity.z += particles[i].force.z / particles[i].mass;

      particles[i].positions.x += particles[i].velocity.x;
      particles[i].positions.y += particles[i].velocity.y;
      particles[i].positions.z += particles[i].velocity.z;
    }
  }


  fclose(fp);
  return 0; 
}
