#!/bin/bash

# define the parameters
ns=(256 512 1024 2048 4096 8192)
rs=(1 2 4 8 16 32 64)

# load the modules
module purge
module load openmpi/3.1.6-gcc-12.2.0-d2gmn55

# sequential
for n in "${ns[@]}"
do
    echo "executing seq-$n"
    export SLURM_OUTPUT="seq-$n.txt"
    sbatch --wait ./seq-job-script.slurm $n
done

# mpi
for n in "${ns[@]}"
do
    for r in "${rs[@]}"
    do
        echo "executing mpi-$n-$r"
        export SLURM_NTASKS=$r
        export SLURM_OUTPUT="mpi-$n-$r.txt"
        sbatch --wait ./mpi-job-script.slurm $n $r
    done
done