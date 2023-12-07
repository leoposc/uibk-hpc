CC=gcc
MPICC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra -lm -O3 -ffast-math -march=native -g3

.PHONY: all
all: mandelbrot_seq mandelbrot_mpi

.PHONY: clean
clean:
	$(RM) mandelbrot_seq
	$(RM) mandelbrot_mpi

mandelbrot_seq:
	$(CC) $(CFLAGS) mandelbrot_seq.c -o mandelbrot_seq

mandelbrot_mpi:
	$(MPICC) $(CFLAGS) mandelbrot_mpi.c -o mandelbrot_mpi