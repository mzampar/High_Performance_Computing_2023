#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
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
out_csv="./results/mpi_weak_scaling.csv"

# Number of repetitions
repetitions=2


echo "Iteration,Total Tasks,Elapsed Time(s)" > "$out_csv"  # Clear and set header

# Number of OpenMP threads
export OMP_NUM_THREADS=1

# Constant amout of work per worker: C = problem size / number of workers
# Therefore, problem size = C * number of workers 

BASE_ROWS=800
BASE_COLS=1000

tasks_list=({2..48..2})

for ((i=1; i<=$repetitions; i++)); do
    for total_tasks in "${tasks_list[@]}"; do

        let rows=$((BASE_ROWS*total_tasks))

        echo "Running iteration $i with $total_tasks MPI tasks."

        elapsed_time=$(mpirun -np $total_tasks --map-by core ./build/mandelbrot $BASE_COLS $rows -1.5 -1.25 0.5 1.25 65535 | grep "Elapsed time:" | awk '{print $3}')
        
        echo "$i,$total_tasks,$elapsed_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"