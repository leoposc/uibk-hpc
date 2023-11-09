#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MIN_DISTANCE 100
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

  return square(deltaX) + square(deltaY) + square(deltaZ);
}

bool checkValidty(Partciles partcile1, Partciles partcile2) {
  bool validity = true;

  if (getDistanceSquared(partcile1, partcile2) < MIN_DISTANCE) {
    validity = false;
  }

  return validity;
}

Coordinates getUnitVector(Partciles partcile1, Partciles partcile2) {
  Coordinates directionVec;

  double deltaX = partcile2.positions.x - partcile1.positions.x;
  double deltaY = partcile2.positions.y - partcile1.positions.y;
  double deltaZ = partcile2.positions.z - partcile1.positions.z;

  double sumOfDelta = deltaX + deltaY + deltaZ;

  directionVec.x = deltaX / sumOfDelta;
  directionVec.y = deltaY / sumOfDelta;
  directionVec.z = deltaZ / sumOfDelta;


  return directionVec;
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
  if (argc != 3) {
    printf("wrong amount of parameters\n");
    return -1;
  }

  size_t num_particles = atoi(argv[1]);
  size_t time_steps = atoi(argv[2]);

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

    particles[i].mass = 100;

    particles[i].positions.x = rand() % MAX_CORDINATE;
    particles[i].positions.y = rand() % MAX_CORDINATE;
    particles[i].positions.z = rand() % MAX_CORDINATE;
  }

  double startTime = omp_get_wtime();

  for (size_t t = 0; t < time_steps; t++) {
    storePositions(particles, num_particles, fp);
    #pragma omp parallel for
    for (size_t i = 0; i < num_particles; i++) {
      particles[i].force.x = 0;
      particles[i].force.y = 0;
      particles[i].force.z = 0;
      for (size_t j = 0; j < num_particles; j++) {
        //adjust force of particle i impacted by all other particles
        if(i == j || checkValidty(particles[i], particles[j])) {
          // skip itslef, or particle on its place
          continue;
        }
        //avoid dividing by 0
        // fix direciton with last multiplyer
        Coordinates unitVector = getUnitVector(particles[i], particles[j]);
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

  double endTime = omp_get_wtime();
  printf("time: %f\n", endTime-startTime);

  fclose(fp);
  return 0; 
}
