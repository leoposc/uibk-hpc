CC=gcc
MPICC=mpicc
CFLAGS=-std=gnu11 -Wall -Wextra -lm -O3 -ffast-math -march=native -g3

.PHONY: all
all: mandelbrot_seq mandelbrot_mpi mandelbrot_mpi_balanced mandelbrot_diffusion_mpi

.PHONY: clean
clean:
	$(RM) mandelbrot_seq
	$(RM) mandelbrot_mpi
	$(RM) mandelbrot_mpi_balanced
	$(RM) mandelbrot_diffusion_mpi

mandelbrot_seq:
	$(CC) $(CFLAGS) mandelbrot_seq.c -o mandelbrot_seq

mandelbrot_mpi:
	$(MPICC) $(CFLAGS) mandelbrot_mpi.c -o mandelbrot_mpi

mandelbrot_mpi_balanced:
	$(MPICC) $(CFLAGS) mandelbrot_mpi_balanced.c -o mandelbrot_mpi_balanced

mandelbrot_diffusion_mpi:
	$(MPICC) $(CFLAGS) mandelbrot_diffusion_mpi.c -o mandelbrot_diffusion_mpi