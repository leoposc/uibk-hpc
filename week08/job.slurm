#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

mpiexec $1 $2 $3 $4
