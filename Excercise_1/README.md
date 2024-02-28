Cozzini Project


module load openMPI/4.1.5/icc/2021.7.1


mkdir $HOME/.local/programs
cd programs
wget https://mvapich.cse.ohio-state.edu/download/mvapich/osu-micro-benchmarks-7.3.tar.gz

./configure —prefix=/u/dssc/mzampar/.local/modules CC=/opt/programs/openMPI/4.1.5/bin/mpicc CXX=/opt/programs/openMPI/4.1.5/bin/mpicxx



cd bcast/bcast_linear
sbatch bcast_linear_getdata.sh
