#!/bin/bash

#SBATCH --nodes=4 
#SBATCH --ntasks-per-node=1
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load necessary modules
module load openMPI/4.1.5/gnu/12.2.1

# Compile the program
mpicx -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
echo "Nodes,Threads,Time(s)\n" >> "$out_csv"

# Number of repetitions
repetitions=10

# Set environment variables
export OMP_NUM_THREADS=1

# MPI scaling
echo "MPI scaling:"
echo "Nodes,Threads,Time(s)\n" >> "$out_csv"

for nodes in 2 3 4; do


for ((threads=2; threads<=24; threads+=2)); do
    total_time=0
    times=()

    for ((i=1; i<=$repetitions; i++)); do
        echo "Running repetition $i with $nodes MPI threads..."
        srun -n nodes ./build/mandelbrot 1000 1000 -2 -2 2 2 1000
    done

done

echo "Execution completed. Results saved to $out_csv"