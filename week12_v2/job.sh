#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --ntasks=2
#SBATCH --output=output.txt
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=12
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive


# mpiexec  bin/multipleFiles 1 -np 12 --mca fs_ufs_lock_algorithm 1
mpiexec bin/individualPointer 1 -np 12 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/sharedPointerNonCollective 1 -np 2 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/sharedPointerCollective 1 -np 2 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/exercise02 1 -np 2 --mca fs_ufs_lock_algorithm 1
