#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name test
# Redirect output stream to this file
#SBATCH --output=output.log
#SBATCH --ntasks=6
# Maximum number of tasks (=processes) to start per node
#SBATCH --exclusive

module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

/scratch/c703429/software/must-1.9.1/bin/mustrun -np 5 ./bin/heat_stencil_2D_mpi_nb
# echo "program 2"
# mpiexec -np 2 ./example_2