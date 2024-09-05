#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=12
#SBATCH --time=00:30:00
#SBATCH --partition=THIN
#SBATCH --exclusive

module load openMPI/4.1.5/gnu
cd osu-micro-benchmarks-7.3/

./configure --prefix=$HOME/.local CC=/opt/programs/openMPI/4.1.6/bin/mpicc CXX=/opt/programs/openMPI/4.1.6/bin/mpicxx
make
make install