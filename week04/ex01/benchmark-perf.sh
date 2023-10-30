#!/bin/bash

executable="heat_stencil_2D_mpi"
ranks=96
n=768

sbatch \
    --ntasks=$ranks \
    --wait \
    "./job-perf.slurm" "./bin/${executable}" $n