#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=128
#SBATCH --time=01:00:00
#SBATCH --partition=EPYC
#SBATCH --job-name=omp_weak_scaling
#SBATCH --error=omp_weak_scaling_%j.err
#SBATCH --error=omp_weak_scaling_%j.out
#SBATCH --exclusive
#SBATCH -A dssc


# Load modules
module load openMPI/4.1.6/gnu/14.2.1


# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c ./build/utils.c -Ibuild -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/omp_weak_scaling.csv"

# Number of repetitions
repetitions=5

# Constant amout of work per worker: C = problem size / number of workers
# Therefore, problem size = C * number of workers 

BASE_ROWS=1000
BASE_COLS=1000

echo "Iteration,Threads,Elapsed Time(s)" > "$out_csv"  # Clear and set header

lst1=(1 2 4 8)
lst2=({16..128..8})
threads_list=("${lst1[@]}" "${lst2[@]}")

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