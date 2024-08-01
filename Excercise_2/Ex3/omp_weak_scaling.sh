#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]
#SBATCH --exclude thin006

# Load modules
module load openMPI/4.1.5/gnu/12.2.1


# Compile the program
mpicc -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./results/omp_weak_scaling.csv"

# Number of repetitions
repetitions=2

BASE_ROWS=800
BASE_COLS=1000
max_procs=48


echo "Iteration,Threads,Elapsed Time(s)" > "$out_csv"  # Clear and set header

threads_list=({2..24..2})

for ((i=1; i<=$repetitions; i++)); do
    for threads in "${threads_list[@]}"; do

        let rows=$((BASE_ROWS*threads))

        echo "Running repetition $i with $threads OMP threads..."
        export OMP_NUM_THREADS=$threads
        elapsed_time=$(mpirun -np 1 --map-by socket --bind-to socket ./build/mandelbrot $BASE_COLS $rows -1.5 -1.25 0.5 1.25 255 | grep "Elapsed time:" | awk '{print $3}')

        echo "$i,$threads,$elapsed_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"
