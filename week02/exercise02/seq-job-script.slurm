#!/bin/bash

#SBATCH --partition=lva
#SBATCH --job-name=heat_stencil_1D_seq
#SBATCH --ntasks=1
#SBATCH --exclusive

module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

srun ./heat_stencil_1D_seq $1