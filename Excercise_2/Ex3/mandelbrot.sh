#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load necessary modules

module load openMPI/4.1.5/icc/2021.7.1

# Set environment variables
export OMP_NUM_THREADS=24

# Compile the program
gcc -o ./build/main mandelbrot_omp.c -lm -fopenmp

# Run the program
./build/main 10000 10000 10000 10000 -2.25 -1.5 0.75 1.5 10000