#!/bin/bash

# executables
es=('n_body_collective_mpi')

# ranks
rs=(10)

# time steps
ts=(100)

# problem sizes
ns=(5000)

# results for this exeuction
out_dir=$(date +%s)

for e in "${es[@]}"
do
    for r in "${rs[@]}"
    do
        for n in "${ns[@]}"
        do
            for t in "${ts[@]}"
            do
                name="${e}_${r}_${n}_${t}"
                out="./benchmarks/${out_dir}/${name}.out"
                echo "executing $name"
                sbatch \
                    --job-name=$name \
                    --output=$out \
                    --ntasks=$r \
                    --wait \
                    "./job.slurm" "./bin/${e}" $n $t
            done
        done
    done
done
