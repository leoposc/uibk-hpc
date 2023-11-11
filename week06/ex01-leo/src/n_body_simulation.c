#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>


typedef float* Vector;

/**
 * @brief   initialize the position, velocity and mass of the particles
 * @param   A  array of length P * 7 (x-coord, y-coord, z-coord,
 *              velocity_x, veloctiy_y, velocity_z, m)
 * @param   P  number of particles
 */ 
void initialize(float *A, const unsigned int P);

/**
 * @brief   compute distance between two particles in a 3D space
 * @param   p_1 coordinates of particle 1 stored in a float array of length 3 (input)
 * @param   p_2 coordinates of particle 2 stored in a float array of length 3 (input)
 * @param   distance distance between the two particles (output)
 * @param   direction_vector direction vector between the two particles (output)
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
    const float *f, const float *direction_vector);

/**
 * @brief   update the position of particle
 * @param   pos position
 * @param   v   velocity
 */
void compute_position(float *pos, const float *v);

/**
 * 
 */
void save_position(const float *pos, const unsigned int P);

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
    unsigned int T = 10;
    unsigned int P = 5;
    if(argc > 1) {
        P = atoi(argv[1]);
    }

    // create the arrays
    Vector A = createVector(P);
    Vector B = createVector(P);
    
    // --------------------------- INITIALIZATION -----------------------------
    // initialize the position, velocity and mass of the particles
    initialize(B, P);
    
    // --------------------------- COMPUTATION --------------------------------   
    // simulate the time steps
    for(unsigned int t = 0; t < T; t++) {
        // loop through particles
        for(unsigned int i = 0; i < P * 7; i += 7) {
            //copy data from B to A
            memcpy(A, B, P * 7 * sizeof(float));
            // loop through particles
            for(unsigned int j = 0; j < P * 7; j += 7) {
                // skip if particle is the same
                if(i == j) {
                    continue;
                }

                // compute distance between particles
                float force;
                float distance;
                float direction_vector[3];
                // use the old position of particle i and j to compute the distance
                compute_distance(&A[i], &A[j], &distance, direction_vector);

                // compute force between particles using the constant mass (A or B)
                compute_force(&B[i + 6], &B[j + 6], &distance, &force);

                // compute velocity of particle i using the updated (B) Velocity and the constant mass (A or B)
                compute_velocity(&B[i + 3], &B[i + 6], &force, direction_vector);

                // update position of particle i using the updated (B) position and velocity
                compute_position(&B[i], &B[i + 3]);                
            }            
        }       

        // save the position of the particles in a file
        save_position(B, P);
    }


    releaseVector(A);
    releaseVector(B);

    return 0;
}

// ----------------------------------------------------------------------
void initialize(float *A, const unsigned int P) {
    for(unsigned int i = 0; i < P * 7; i += 7) {
        // delete any "data.dat" file
        remove("data.dat");
        // initialize position with random numbers between 0 and 1000
        A[i] = rand() % 100;
        A[i + 1] = rand() % 100;
        A[i + 2] = rand() % 100;
        // initiliaze velocity with 0
        A[i + 3] = 0;
        A[i + 4] = 0;
        A[i + 5] = 0;
        // initialize mass with random numbers between 1 and 100
        A[i + 6] = rand() % 100 + 1;
    }
}

// ----------------------------------------------------------------------
void compute_distance(const float *p_1, const float *p_2,
    float *distance, float *direction_vector) {
        
    float x = p_1[0] - p_2[0];
    float y = p_1[1] - p_2[1];
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

// ----------------------------------------------------------------------
void compute_position(float *pos, const float *v) {
    pos[0] += v[0];
    pos[1] += v[1];
    pos[2] += v[2];
}

// ----------------------------------------------------------------------
void save_position(const float *pos, const unsigned int P) {
    // append the position of the particles to the file
    FILE *fp = fopen("data.dat", "a");
    for(unsigned int i = 0; i < P * 7; i += 7) {
        fprintf(fp, "%f %f %f\n", pos[i], pos[i + 1], pos[i + 2]);
    }
    fprintf(fp, "\n\n");
    fclose(fp);
}


// ----------------------------------------------------------------------
Vector createVector(int N) {
    return malloc(sizeof(float) * N * 7);
}

// ----------------------------------------------------------------------
void releaseVector(Vector m) {
    free(m);
} 