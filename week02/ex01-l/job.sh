#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name week02
# Redirect output stream to to command
#SBATCH --output=output.log
# Maximum number of tasks (=processes) to start in total
#SBATCH --ntasks=64
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=12

# mpiexec -np $SLURM_NTASKS ./pi_mpi 1000000000

module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

mpiexec -np $SLURM_NTASKS ./heat_stencil_1D_mpi 4096
