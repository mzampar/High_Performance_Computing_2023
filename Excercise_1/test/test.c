#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// function to print hello from each process
void print_hello(int rank, int size) {
    printf("Hello from process %d of %d\n", rank, size);
}

int main(int argc, char *argv[]) {
    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    // Get the rank (ID) of the current process and the total number of processes
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Call the function to print hello from each process
    print_hello(rank, size);

    // Finalize MPI environment
    MPI_Finalize();
    return 0;
}
