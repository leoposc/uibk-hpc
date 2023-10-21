#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name test
# Redirect output stream to this file
#SBATCH --output=output.log
# Maximum number of tasks (=processes) to start in total
#SBATCH --ntasks=20
# Maximum number of tasks (=processes) to start per node
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

srun ./heat_stencil_2D_mpi 768