#!/bin/bash

# executables
es=('mandelbrot_mpi')

# ranks
rs=(1 12 24 36 48 60 72 80 90)

# results for this exeuction
out_dir=$(date +%s)

for e in "${es[@]}"
do
    for r in "${rs[@]}"
    do
        x=3840
        y=2160
        name="${e}_${r}_${x}_${y}"
        out="./benchmarks/${out_dir}/${name}.out"
        echo "executing $name"
        sbatch \
            --job-name=$name \
            --output=$out \
            --ntasks=$r \
            --wait \
            "./job.slurm" "./${e}" $x $y
    done
done
