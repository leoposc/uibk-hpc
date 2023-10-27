#!/bin/bash

# executables
#es=('heat_stencil_1D_mpi' 'heat_stencil_1D_mpi_nb' 'heat_stencil_2D_mpi' 'heat_stencil_2D_mpi_nb')
es=('heat_stencil_1D_mpi')

# ranks
#rs=(16 32 96)
rs=(8 16)

# problem sizes
#ns=(192 288 384 480 576 672 768)
ns=(1024 2048)

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
        done
    done
done