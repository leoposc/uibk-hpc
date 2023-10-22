#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name=heat_stencil_2D_mpi
#SBATCH --exclusive
#SBATCH --ntasks-per-node=12

module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

mpiexec ./heat_stencil_2D_mpi $1