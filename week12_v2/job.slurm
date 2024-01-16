#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

mpiexec $1 $2 --mca fs_ufs_lock_algorithm 1