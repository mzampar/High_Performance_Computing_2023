#!/bin/bash

#SBATCH --nodes=4
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=mpi_strong_scaling
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]
#SBATCH --exclude thin006

# Load modules
module load openMPI/4.1.5/gnu/12.2.1

# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/mpi_strong_scaling.csv"

# Number of repetitions
repetitions=1

echo "Iteration,Total Tasks,Elapsed Time(s)" > "$out_csv"  # Clear and set header

# Number of OpenMP threads
export OMP_NUM_THREADS=1

tasks_list=({2..96..2})

echo "Running MPI strong scaling."

for ((i=1; i<=$repetitions; i++)); do
    for total_tasks in "${tasks_list[@]}"; do

        echo "Running iteration $i with $total_tasks MPI tasks."

        elapsed_time=$(mpirun -np $total_tasks ./build/mandelbrot 1000 26880 -1.5 -1.25 0.5 1.25 255 | grep "Elapsed time:" | awk '{print $3}')
        
        echo "$i,$total_tasks,$elapsed_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"