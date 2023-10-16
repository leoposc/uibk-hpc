#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <omp.h>
#include <string.h>

size_t calc_dellanoy(int m, int n) {
  if (m == 0 || n == 0) {
    return 1;
  }
  
  return calc_dellanoy(m-1, n) + calc_dellanoy(m-1, n-1) + calc_dellanoy(m, n-1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("pls enter the size of the square");
    return 0;
  }
  
  int N = atoi(argv[1]);
  
  
	double startTime = omp_get_wtime();
  printf("num: %lu\n", calc_dellanoy(N, N));
	double endTime = omp_get_wtime();
	printf("time: %2.4f seconds\n", endTime-startTime);
}
