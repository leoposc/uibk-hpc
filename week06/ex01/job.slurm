#!/bin/bash

#SBATCH --partition=lva
#SBATCH --ntasks-per-node=12
#SBATCH --exclusive

srun $1 $2 $3
