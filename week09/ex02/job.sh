#!/bin/bash

program=$1
cores=$2
samples=$3
idle=$4

echo "Binary:" $program 
echo "Number of cores:" $cores 
echo "Number of samples:" $samples

if [ $idle -eq 0 ]
then  
   perf stat -e power/energy-pkg/ mpiexec --oversubscribe --use-hwthread-cpus --mca mpi_yield_when_idle 0 -n $cores $program $samples > "out.data"
else
   echo "YIELD WHEN IDLE IS SET TO TRUE"
   perf stat -e power/energy-pkg/ mpiexec --oversubscribe --use-hwthread-cpus --mca mpi_yield_when_idle 1 -n $cores $program $samples > "out.data"
fi 