#!/bin/bash

# executables
es=('n_body_collective_mpi' 'n_body_p2p_mpi')

# ranks
rs=(1 12 48 96)

# time steps
ts=(100)

# problem sizes
ns=(1248 2496 4992 9984)

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
