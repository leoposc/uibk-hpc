#!/bin/bash

# Execute job in the partition "lva" unless you have special requirements.
#SBATCH --partition=lva
# Name your job to be able to identify it later
#SBATCH --ntasks=2
# Maximum number of tasks (=processes) to start per node
#SBATCH --ntasks-per-node=12
# Enforce exclusive node allocation, do not share with other jobs
#SBATCH --exclusive

# executables
es=('individualPointer' 'multipleFiles' 'sharedPointerCollective' 'sharedPointerNonCollective' 'exercise02')

# ranks
# rs=(2)
rs=(12 24 48 96)

# problem sizes in MB
# ns=(1)
ns=(8 16 32 64)

# results for this exeuction
out_dir=$(date +%s)

for e in "${es[@]}"
do
    for n in "${ns[@]}"
    do
        for r in "${rs[@]}"
        do
            name="${e}_${r}_${n}"
            out="./benchmarks/${out_dir}/${name}.out"
            echo "executing $name"
            sbatch \
                --job-name=$name \
                --output=$out \
                --ntasks=$r \
                --wait \
                "./job.slurm" "./bin/${e}" $n

            rm -f $SCRATCH/output*
        done
    done
done


# mpiexec -np 12 bin/multipleFiles
# mpiexec -np 12 bin/individualPointer 1 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/sharedPointerNonCollective 1 -np 2 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/sharedPointerCollective 1 -np 2 --mca fs_ufs_lock_algorithm 1
# mpiexec bin/exercise02 1 -np 2 --mca fs_ufs_lock_algorithm 1
