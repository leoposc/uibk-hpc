#!/bin/bash

# executables
es=('Nbody')

# time steps
# ts=(50 100 200)
ts=(100)

# problem sizes
# ns=(1000 2000 5000)
ns=(192 384 768 1536 3072 4992)

# results for this exeuction
out_dir=$(date +%s)

for e in "${es[@]}"
do
    for n in "${ns[@]}"
    do
        for t in "${ts[@]}"
        do
            name="${e}_${t}_${n}"
            out="./benchmarks/${out_dir}/${name}.out"
            echo "executing $name"
            sbatch \
                --job-name=$name \
                --output=$out \
                --ntasks=1 \
                --wait \
                "./job.slurm" "./bin/${e}" $n $t
        done
    done
done
