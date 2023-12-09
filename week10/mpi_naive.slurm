#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name mpi_naive_mandelbrot
#SBATCH --ntasks=96
#SBATCH --exclusive

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55
mpiexec -n $SLURM_NTASKS mandelbrot_mpi 3840 2160
