#!/bin/bash

# executables
es=('n_body_collective_aos')

# ranks
rs=(96)

# time steps
ts=(100 1000 5000 10000 20000)

# problem sizes
ns=(1920)

# benchmark (0 = no, 1 = yes)
b=1

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
                name="${e}_${r}_${n}_${t}_${b}"
                out="./benchmarks/${out_dir}/${name}.out"
                echo "executing $name"
                sbatch \
                    --job-name=$name \
                    --output=$out \
                    --ntasks=$r \
                    --wait \
                    "./job.slurm" "./bin/${e}" $n $t $b
            done
        done
    done
done
