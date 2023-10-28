#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

perf record -g mpiexec $1 $2
