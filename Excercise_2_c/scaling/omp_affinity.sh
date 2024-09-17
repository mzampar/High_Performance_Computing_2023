#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=64
#SBATCH --time=01:00:00
#SBATCH --partition=EPYC
#SBATCH --job-name=omp_strong_scaling
#SBATCH --error=omp_strong_scaling_%j.err
#SBATCH --error=omp_strong_scaling_%j.out
#SBATCH --exclusive
#SBATCH -A dssc

# 64 threads per node because one process can be bound to one socket/cpu
# (it is not possible to bind a process to an entire node)
# Therefore, 64 threads per node because there are 64 cores per socket in an epyc node
# We use 128 threads to check how SMT works

# Load modules
module load openMPI/4.1.6/gnu/14.2.1

#!/bin/bash

# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c ./build/utils.c -Ibuild -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/omp_affinity.csv"

echo "Iteration,Threads,OMP_PLACES,OMP_PROC_BIND,Elapsed Time(s),Computation Time(s),Write Time(s)" > "$out_csv"  # Clear and set header

# List of thread counts and affinities to test
lst1=(1 2 4 8)
lst2=({16..64..8})
threads_list=("${lst1[@]}" "${lst2[@]}")
affinity_list=("cores" "sockets" "threads" "none")
bind_list=("close" "spread" "none")


echo "Running OpenMP strong scaling with different thread affinities."

for threads in "${threads_list[@]}"; do
    for places in "${affinity_list[@]}"; do
        for bind in "${bind_list[@]}"; do
            echo "Running repetition $i with $threads OMP threads, OMP_PLACES=$places, OMP_PROC_BIND=$bind..."
            export OMP_NUM_THREADS=$threads
            export OMP_PLACES=$places
            export OMP_PROC_BIND=$bind
            output=$(mpirun -np 1 --map-by socket --bind-to socket ./build/mandelbrot 25000 25000 -1.5 -1.25 0.5 1.25 255)

            elapsed_time=$(echo "$output" | grep "Elapsed time:" | awk '{print $3}')
            computation_time=$(echo "$output" | grep "Computation time:" | awk '{print $3}')
            write_time=$(echo "$output" | grep "Write time:" | awk '{print $3}')

            echo "$i,$threads,$places,$bind,$elapsed_time,$computation_time,$write_time" >> "$out_csv"
        done
    done
    done


echo "Execution completed. Results saved to $out_csv"