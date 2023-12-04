#!/bin/bash

for i in {1..5}
do
  bash job.sh bin/pi_mpi 32 400000000 0
done