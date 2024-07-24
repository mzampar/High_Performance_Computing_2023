#!/bin/bash

#SBATCH --nodes=1
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
mpicc -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./results/mandelbrot_omp_execution_times.csv"

# Number of repetitions
repetitions=10

# OMP scaling
echo "OMP scaling:"
echo "Nodes,Threads,Time(s)" > "$out_csv"  # Clear and set header

for ((threads=2; threads<=24; threads+=2)); do
    total_time=0

    for ((i=1; i<=$repetitions; i++)); do
        echo "Running repetition $i with $threads OMP threads..."
        export OMP_NUM_THREADS=$threads
        elapsed_time=$(srun --exclusive ./build/mandelbrot 1000 1000 -2 -2 2 2 1000 | grep "Elapsed time:" | awk '{print $3}')
        echo "1,$threads,$elapsed_time" >> "$out_csv"
    done

done

echo "Execution completed. Results saved to $out_csv"
