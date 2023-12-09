#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=1
#SBATCH --exclusive

mpiexec $1 $2 $3
