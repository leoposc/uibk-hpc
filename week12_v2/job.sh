#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name test
# Redirect output stream to this file
#SBATCH --output output.txt
# /dev/null
#SBATCH --ntasks=12
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=12
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

# mpiexec -np 12 bin/multipleFiles
# mpiexec -np 12 bin/individualPointer --mca fs_ufs_lock_algorithm 1
mpiexec -np 12 bin/sharedPointerNonCollective --mca fs_ufs_lock_algorithm 1
# mpiexec -np 4 bin/sharedPointerCollective --mca fs_ufs_lock_algorithm 1
