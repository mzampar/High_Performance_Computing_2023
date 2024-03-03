#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load the openMPI module
module load openMPI/4.1.5/icc/2021.7.1

# Compile the Mandelbrot_omp.c file
mpicc -o main mandelbrot_omp.c -lm -fopenmp

# Run the compiled executable
srun -n 24 ./main 10000 10000 -2 -2 2 2 10000 