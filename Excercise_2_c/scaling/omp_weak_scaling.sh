#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=12
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=omp_weak_scaling
#SBATCH --error=omp_strong_scaling_%j.err
#SBATCH --error=omp_strong_scaling_%j.out
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]
#SBATCH --exclude thin006

# Load modules
module load openMPI/4.1.5/gnu/12.2.1


# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/omp_weak_scaling.csv"

# Number of repetitions
repetitions=1

# Constant amout of work per worker: C = problem size / number of workers
# Therefore, problem size = C * number of workers 

BASE_ROWS=10000
BASE_COLS=10000

echo "Iteration,Threads,Elapsed Time(s)" > "$out_csv"  # Clear and set header

threads_list=({2..24..2})

echo "Running OpenMP weak scaling."

for ((i=1; i<=$repetitions; i++)); do
    for threads in "${threads_list[@]}"; do

        rows=$(echo "$BASE_ROWS * sqrt($threads)" | bc -l)
        cols=$(echo "$BASE_COLS * sqrt($threads)" | bc -l)

        echo "Running repetition $i with $threads OMP threads..."
        export OMP_NUM_THREADS=$threads
        export OMP_PLACES=cores
        export OMP_PROC_BIND=close
        elapsed_time=$(mpirun -np 1 --map-by socket --bind-to socket ./build/mandelbrot $cols $rows -1.5 -1.25 0.5 1.25 255 | grep "Elapsed time:" | awk '{print $3}')

        echo "$i,$threads,$elapsed_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"
