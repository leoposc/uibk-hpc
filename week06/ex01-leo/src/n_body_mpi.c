#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>


#define TIMESTEPS 100
#define NUM_PARTICLES 1000
#define MIN_DISTANCE 0.00000001
#define MAX_COORDINATE 100
#define MASS 0.1

typedef struct {
    float x;
    float y;
    float z;
} Coordinates;

typedef struct {
    Coordinates position;
    Coordinates velocity;
    float mass;
} Particle;

typedef Particle *Vector;

Vector createVector(int P) {
    return malloc(sizeof(Particle) * P);
}

void releastVector(Vector v) {
    free(v);
}

float square(float num) {
    return num * num;
}

void initialize(Particle* particles, unsigned int num_particles) {
    srand(time(NULL));
    for (size_t i = 0; i < num_particles; i++) {
        particles[i].position.x = (rand() / (1.0 * RAND_MAX)) * MAX_COORDINATE;
        particles[i].position.y = (rand() / (1.0 * RAND_MAX)) * MAX_COORDINATE;
        particles[i].position.z = (rand() / (1.0 * RAND_MAX)) * MAX_COORDINATE;
        particles[i].velocity.x = 0;
        particles[i].velocity.y = 0;
        particles[i].velocity.z = 0;
        // particles[i].mass = rand() % MASS;
        particles[i].mass = MASS;
    }
}

void compute_distance(const Particle* particle1, const Particle* particle2,
    float* distance, Coordinates* direction_vector) {
    
    float deltaX = particle2->position.x - particle1->position.x;
    float deltaY = particle2->position.y - particle1->position.y;
    float deltaZ = particle2->position.z - particle1->position.z;

    *distance = sqrt(square(deltaX) + square(deltaY) + square(deltaZ));
    direction_vector->x = deltaX / *distance;
    direction_vector->y = deltaY / *distance;
    direction_vector->z = deltaZ / *distance;    
    
    assert(*distance >= 0);
    }

void compute_force(const float* mass1, const float* mass2, const float* distance, float* force) {
    *force = (*mass1) * (*mass2) / square(*distance);
}


void compute_velocity(Particle* particle, const float* tot_x, const float* tot_y, const float* tot_z) {
    particle->velocity.x += (*tot_x) / particle->mass;
    particle->velocity.y += (*tot_y) / particle->mass;
    particle->velocity.z += (*tot_z) / particle->mass;    
}

void compute_position(Particle* particle) {
    particle->position.x += particle->velocity.x;
    particle->position.y += particle->velocity.y;
    particle->position.z += particle->velocity.z;
}

void save_position(const Particle* p, const unsigned int num_particles, FILE* fp) {
    for (size_t i = 0; i < num_particles; i++) {
        fprintf(fp, "%f %f %f\n", p[i].position.x, p[i].position.y, p[i].position.z);
        
    }
    fprintf(fp, "\n");
    fprintf(fp, "\n");
}



int main(int argc, char** argv) {

    // --------- I/ O setup ------------
    FILE *fp = fopen("data.dat", "w");
    if(fp == NULL) {
        printf("Error opening file\n");
        return -1;
    }
    // ---------------------------------


    // ---------setup MPI----------------
    MPI_Init(&argc, &argv);
    MPI_Comm comm;
    MPI_Comm_dup(MPI_COMM_WORLD, &comm);
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    MPI_Request request;
    // introduce MPI to the defined structures
    MPI_Datatype MPI_COORDINATES;
    MPI_Datatype MPI_PARTICLE;
    MPI_Type_contiguous(3, MPI_FLOAT, &MPI_COORDINATES);
    MPI_Type_commit(&MPI_COORDINATES);
    MPI_Type_contiguous(6, MPI_FLOAT, &MPI_PARTICLE);
    MPI_Type_commit(&MPI_PARTICLE);
    // ----------------------------------
    
    // ----parameters setup------------
    size_t num_particles;
    size_t time_steps;
    if (argc != 3) {
        num_particles = NUM_PARTICLES / size;
        time_steps = TIMESTEPS;
    } else {
        num_particles = atoi(argv[1]) / size;
        time_steps = atoi(argv[2]);
    }
    // --------------------------------

    // ---------setup particles----------
    Vector broadcast_buf = createVector(num_particles);
    // Vector all_particles = createVector(num_particles * size);

    Vector particlesA = createVector(num_particles);
    Vector particlesB = createVector(num_particles);
    initialize(particlesA, num_particles);    
    memcpy(particlesB, particlesA, sizeof(Particle) * num_particles);
    // ----------------------------------

    for (size_t t = 0; t < time_steps; t++) {
        if (rank == 0) {
            // TODO: Save all particles, not only the ones of rank 0
            save_position(particlesA, num_particles, fp);
        }

        // TODO: Broadcast particles
        memcpy(broadcast_buf, particlesA, sizeof(Particle) * num_particles);

        for (int r = 0; r < size; r++) {

            MPI_Ibcast(broadcast_buf, num_particles, MPI_PARTICLE, r, comm, &request);

            for (size_t i = 0; i < num_particles; i++) {
                float total_force_x = 0.0;
                float total_force_y = 0.0;
                float total_force_z = 0.0;

                for (size_t j = 0; j < num_particles; j++) {
                    if (j == i) { // not correct anymore?
                        continue;
                    }

                    float force, distance;
                    Coordinates direction_vector;
                    compute_distance(&particlesA[i], &particlesA[j], &distance, &direction_vector);

                    if (distance < MIN_DISTANCE) {
                        continue;
                    }

                    compute_force(&particlesA[i].mass, &particlesA[j].mass, &distance, &force);

                    // Accumulate forces
                    total_force_x += force * direction_vector.x;
                    total_force_y += force * direction_vector.y;
                    total_force_z += force * direction_vector.z;
                } // end of particle j loop

            // Update velocity and position after computing all forces
            compute_velocity(&particlesB[i], &total_force_x, &total_force_y, &total_force_z);
            compute_position(&particlesB[i]);


            } // end of particle i loop
    
                // Wait for broadcast to finish
            MPI_Wait(&request, MPI_STATUS_IGNORE);

            // copy broadcast_buf to particlesA
            memcpy(particlesA, broadcast_buf, sizeof(Particle) * num_particles);
            
        } // end of rank loop

        // copy particlesB to particlesA
        memcpy(particlesA, particlesB, sizeof(Particle) * num_particles);

    } // end of time step loop

    // --------------clean up--------------------
    fclose(fp);
    releastVector(particlesA);
    releastVector(particlesB);
    MPI_Type_free(&MPI_COORDINATES);
    MPI_Type_free(&MPI_PARTICLE);
    MPI_Finalize();
    // ------------------------------------------
    return 0;

}
