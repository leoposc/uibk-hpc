#ifndef COMMON_H
#define COMMON_H

/**
 * The number of characters on both directions of the
 * temparature print function.
 */
#define RESOLUTION 120

/**
 * Macro used to fetch a single element of a matrix.
 */
#define IDX(i, j, N) ((i)*N + (j))

/**
 * Defines the type of entries in a matrix.
 */
typedef double value_t;

/**
 * Defines a alias for a matrix.
 */
typedef value_t* Matrix;

/**
 * Allocates memory for a matrix.
 */
Matrix createMatrix(int N, int M);

/**
 * Interpretes the values of the matrix as temperature
 * and prints them to stdout.
 */
void printTemperature(Matrix m, int N);

/**
 * Releases the memory of a matrix.
 */
void releaseMatrix(Matrix m);

#endif