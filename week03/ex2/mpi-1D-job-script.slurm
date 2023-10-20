#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

mpiexec ./heat_stencil_1D_mpi $1