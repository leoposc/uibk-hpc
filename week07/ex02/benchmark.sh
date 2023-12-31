#!/bin/bash

# executables
es=('n_body_p2p_mpi' 'n_body_p2p_improved_mpi' 'n_body_collective_mpi') #_improved

# ranks
rs=(96)

# time steps
ts=(100)

# problem sizes
ns=(4992)

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
                    "./job.slurm" "./bin/${e}" $n $t 1
            done
        done
    done
done
