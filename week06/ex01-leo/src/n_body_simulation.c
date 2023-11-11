#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


typedef float* Vector;

/**
 * @brief   compute distance between two particles in a 3D space
 * @param   p_1 coordinates of particle 1 stored in a float array of length 3
 * @param   p_2 coordinates of particle 2 stored in a float array of length 3
 * @param   distance distance between the two particles
 * @param   direction_vector direction vector between the two particles
 */
void compute_distance(const float *p_1, const float *p_2,
    float *distance, float *direction_vector);

/**
 * @brief   compute the gravitational force between two particles
 *          force = G * (mass_1 * mass_2) / radius^2 
 *          where G is the gravitational constant (for simplicity: G = 1)
 * @param   m_1 mass particle 1
 * @param   m_2 mass particle 2
 * @param   r   distance between the two particles
 * @return  gravitational force
 */ 
void compute_force(const float *m_1, const float *m_2, const float *r, float *force);

/**
 * @brief   update the velocity of particle
 * @param   v   velocity
 * @param   f   force 
 * @param   m   mass 
 */
void compute_velocity(float *velocity, const float *m,
    const float *f, const float *direction_vector)

/**
 * @brief   update the position of a particle in a three dimensional space
 * @param   p   position of the particle (x, y, z)
 * @param   v   velocity of the particle
 * @param   d   direction vector of the particle
 */
void update_position(const float *pos, const float *v, const float *direction_vector, float *updated_pos);

/**
 * @brief   initialize the position and velocity of a particle
 * @param   A   array of length 7 * P containing the position, velocity and mass of a particle
 * @param   B   array of length 7 * P containing the position, velocity and mass of a particle
 */ 
void initialize(const float *A, float *B, int P);

/**
 * @brief   create a vector of length N * 5
 * @param   N   length of the vector
 * @return  vector of length N * 5
 */
Vector createVector(int N);

/**
 * @brief   release the memory of a vector
 * @param   m   vector to be released
 */
void releaseVector(Vector m);


int main(int argc, char** argv) {

    // parsing optional input parameter (problem size)
    int T = 100;
    int P = 500;
    if(argc > 1) {
        P = atoi(argv[1]);
    }

    // create the arrays
    Vector A = createVector(P);
    Vector B = createVector(P);

    // TODO: initialize the arrays with random values

    
    // --------------------------- COMPUTATION --------------------------    
    // simulate the time steps
    for(int t = 0; t < T; t++) {
        // loop through particles
        for(int i = 0; i < P; i++) {
            //copy data from A to B
            memcpy(B, A, P * 7 * sizeof(float));
            // loop through particles
            for(int j = 0; j < P; j++) {
                // skip if particle is the same
                if(i == j) {
                    continue;
                }

                // compute distance between particles
                float force;
                float distance;
                float direction_vector[3];
                compute_distance(&A[i], &A[j], &distance, direction_vector);

                // compute force between particles
                compute_force(&B[i + 6], &B[j + 6], &distance, &force);

                // compute velocity of particle i
                compute_velocity(&B[i + 3], &B[i + 6], &force, direction_vector);

                // update position of particle i
                compute_position(&B[i], &B[i + 3]);

                // TODO: update velocity of particle i
            }
        }

        // swap arrays
        Vector tmp = A;
        A = B;
        B = tmp;
    }


    return 0;
}

// ----------------------------------------------------------------------
void initialize(const float *A, float *B, int P) {
    // copy data from A to B
    memcpy(B, A, P * 7 * sizeof(float));
}

// ----------------------------------------------------------------------
void compute_distance(const float *p_1, const float *p_2,
    float *distance, float *direction_vector) {
        
    float x = p_1[0] - p_2[0];
    float y = p_1[1] - p_2[2];
    float z = p_1[2] - p_2[2];
    float xy = sqrt(x * x + y * y);
    *distance = sqrt(xy * xy + z * z);
    direction_vector[0] = x / (*distance);
    direction_vector[1] = y / (*distance);
    direction_vector[2] = z / (*distance);
}

// ----------------------------------------------------------------------
void compute_force(const float *m_1, const float *m_2, const float *r, float *force) {
    *force = ((*m_1) * (*m_2)) / ((*r) * (*r));
}

// ----------------------------------------------------------------------
void compute_velocity(float *velocity, const float *m,
    const float *f, const float *direction_vector) {

    velocity[0] += ((*f) / (*m)) * direction_vector[0];
    velocity[1] += ((*f) / (*m)) * direction_vector[1];
    velocity[2] += ((*f) / (*m)) * direction_vector[2];
}

// void update_velocity(const float *old_pos, const float *new_pos, float *velocity) {
//     float velocity[0] = new_pos[0] - old_pos[0];
//     float velocity[1] = new_pos[1] - old_pos[1];
//     float velocity[2] = new_pos[2] - old_pos[2];
// }

// ----------------------------------------------------------------------
void compute_position(float *pos, const float *v) {
    pos[0] += v[0];
    pos[1] += v[1];
    pos[2] += v[2];
}

// ----------------------------------------------------------------------
Vector createVector(int N) {
    return malloc(sizeof(float) * N * 5);
}

// ----------------------------------------------------------------------
void releaseVector(Vector m) {
    free(m);
} 