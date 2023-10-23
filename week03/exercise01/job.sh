#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --job-name test
# Redirect output stream to this file
#SBATCH --output=output.log
#SBATCH --ntasks=96
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=12
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

ns=(192 288 384 480 576 672 768)
rs=(16 32 96)

module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

# mpi
for n in "${ns[@]}"
do
    for r in "${rs[@]}"
    do
        echo "executing mpi-$n-$r"
        sbatch --wait ./mpi-job-script.slurm $n $r
    done
done