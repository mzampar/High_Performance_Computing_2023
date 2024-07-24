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
mpicc -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./results/mandelbrot_mpi_execution_times.csv"
echo "Nodes,Threads,Time(s)\n" >> "$out_csv"

# Number of repetitions
repetitions=10


# MPI scaling
echo "MPI scaling:"
echo "Workers,Size,Walltime(s)" > "$out_csv"

# Define the number of threads to use for OpenMP parallelism within each MPI process
threads=1

# List of number of workers
lst=({2..70..2})
num_workers=("1" "${lst[@]}")

for ((i=1; i<=$repetitions; i++)); do
    for workers in "${num_workers[@]}"; do
    # Set the number of MPI processes to the number of workers
    processes=$workers

    echo "Running repetition $i with $processes MPI processes..."

    processes=$workers
    elapsed_time=$(mpirun -np ${processes} --map-by core ./build/mandelbrot ${threads} 1000 1000 -2 -2 2 2 1000 | grep "Elapsed time:" | awk '{print $3}')
    
    if [ -z "$elapsed_time" ]; then
        continue
    fi

    echo "$i,$threads,$elapsed_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"