#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>

// function to print hello from each process
void print_hello(int rank, int size) {
    printf("Hello from process %d\n", rank);
}

int main (int argc, char *argv[]) {
    // Initialize MPI environment for multi-processes
    int mpi_provided_thread_level;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        printf("The threading support level is lesser than that demanded\n");
        MPI_Finalize();
        exit(1);
    }

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    print_hello(rank, size);

    MPI_Finalize();
    return 0;
}