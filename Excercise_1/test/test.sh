#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=test
#SBATCH --exclusive
#SBATCH -A dssc

# Load the openMPI module
module load openMPI/4.1.6/gnu/14.2.1

mpicc -O3 -march=native -o ./test test.c -lm


# Define the range of np values
np_values=$(seq 4 4 48)
# Define the range of map values
map_values="core socket node"

export OMP_NUM_THREADS=1


# Iterate over map and np values
for map in $map_values; do
  for np in $np_values; do
    # Run the mpirun command
    echo "   Running test with map=$map and np=$np"
    mpirun --report-bindings -np $np -map-by $map ./test
  done
done
