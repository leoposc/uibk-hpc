#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <omp.h>
#include <string.h>

#define THESH_PAR 5

size_t calc_dellanoy(int m, int n) {
  if (m == 0 || n == 0) {
    return 1;
  }
  size_t frst_calc;
  size_t snd_calc;

  #pragma omp task shared(frst_calc) if(m > THESH_PAR && n > THESH_PAR)
    frst_calc = calc_dellanoy(m-1, n);

  #pragma omp task shared(snd_calc) if(m > THESH_PAR && n > THESH_PAR)
    snd_calc = calc_dellanoy(m-1, n-1);

  size_t third_calc = calc_dellanoy(m, n-1);
  
  #pragma omp taskwait

  return frst_calc + snd_calc + third_calc;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("pls enter the size of the square\n");
    return 0;
  }
  
  int N = atoi(argv[1]);
  
  
	double startTime = omp_get_wtime();
  size_t result;
  #pragma omp parallel
  {
    #pragma omp single
    result = calc_dellanoy(N, N);
  }
  printf("num: %lu\n", result);
	double endTime = omp_get_wtime();
	printf("time: %2.4f seconds\n", endTime-startTime);
}
