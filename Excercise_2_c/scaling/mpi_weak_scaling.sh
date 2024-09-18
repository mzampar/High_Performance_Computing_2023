#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=128
#SBATCH --time=00:45:00
#SBATCH --partition=EPYC
#SBATCH --job-name=mpi_weak_scaling
#SBATCH --error=mpi_weak_scaling_%j.err
#SBATCH --output=mpi_weak_scaling_%j.out
#SBATCH --exclusive
#SBATCH -A dssc

# Load modules
module load openMPI/4.1.6/gnu/14.2.1

# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/mpi_weak_scaling.csv"

# Number of repetitions
repetitions=5

echo "Iteration,Total Tasks,Elapsed Time(s),Computation Time(s),Gathering Time(s)" > "$out_csv"  # Clear and set header

# Number of OpenMP threads
export OMP_NUM_THREADS=1

# Constant amout of work per worker: C = problem size / number of workers
# Therefore, problem size = C * number of workers 

BASE_ROWS=2000
BASE_COLS=2000

lst1=(1 2 4 8)
lst2=({16..256..8})
tasks_list=("${lst1[@]}" "${lst2[@]}")

echo "Running MPI weak scaling."

for ((i=1; i<=$repetitions; i++)); do
    for total_tasks in "${tasks_list[@]}"; do
        rows=$(echo "$BASE_ROWS * (sqrt($total_tasks)/1+1) " | bc -l)
        cols=$(echo "$BASE_COLS * (sqrt($total_tasks)/1+1)" | bc -l)
        echo "Running iteration $i with $total_tasks MPI tasks."
        output=$(mpirun -np $total_tasks --map-by core --bind-to core ./build/mandelbrot $rows $cols -1.5 -1.25 0.5 1.25 255)
        elapsed_time=$(echo "$output" | grep "Elapsed time:" | awk '{print $3}')
        computation_time=$(echo "$output" | grep "Computation time:" | awk '{print $3}')
        gathering_time=$(echo "$output" | grep "Gathering time:" | awk '{print $3}')        
        echo "$i,$total_tasks,$elapsed_time,$computation_time,$gathering_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"