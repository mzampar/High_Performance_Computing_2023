#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include<omp.h>
#include<string.h>

// Main function to generate the Mandelbrot set image
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size); // num of processes

    if (argc != 8) {
        if (rank == 0) {
            printf("Usage: %s nx ny x_L y_L x_R y_R I_max\n", argv[0]);
        }
        MPI_Finalize();
        exit(1);
    }

    int nx = atoi(argv[1]);  // Number of pixels in x-direction
    int ny = atoi(argv[2]);  // Number of pixels in y-direction
    double x_L = atof(argv[3]);  // Left bound of the complex plane
    double y_L = atof(argv[4]);  // Lower bound of the complex plane
    double x_R = atof(argv[5]);  // Right bound of the complex plane
    double y_R = atof(argv[6]);  // Upper bound of the complex plane
    int I_max = atoi(argv[7]);  // Maximum number of iterations

    int rows_per_process = ny / size;
    int remainder = ny % size;

    // Calculate number of rows each process will handle
    int my_rows = (rank < remainder) ? rows_per_process + 1 : rows_per_process;
    int my_offset = rank * rows_per_process + (rank < remainder ? rank : remainder);

    // Allocate memory for local matrix
    short int *local_matrix = (short int *) malloc(my_rows * nx * sizeof(short int));

    // Compute Mandelbrot set for local rows with OpenMP parallelization
    #pragma omp parallel for schedule(dynamic) shared(local_matrix)
    for (int j = rank; j < ny; j += 1) { // Distribute rows among processes
        for (int i = 0; i < nx; i++) {

            local_matrix[(j - rank) * nx + i] = rank;
        }
    }

// Calculate the displacement and receive counts for MPI_Gatherv
int *recv_counts = malloc(size * sizeof(int));
int *displs = malloc(size * sizeof(int));
for (int i = 0; i < size; i++) {
    recv_counts[i] = my_rows * nx; // Number of elements to receive from each process
    displs[i] = i * my_rows * nx;  // Displacement for each process in the receive buffer
}

// Allocate memory for receiving the gathered matrix on rank 0
short int *gathered_matrix = (rank == 0) ? malloc(ny * nx * sizeof(short int)) : NULL;

// Gather results to rank 0
MPI_Gatherv(local_matrix, my_rows * nx, MPI_SHORT, gathered_matrix, recv_counts, displs, MPI_SHORT, 0, MPI_COMM_WORLD);


// Reorder gathered matrix on rank 0
if (rank == 0) {
    // Output the reordered matrix
    printf("Gathered Matrix:\n");
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            printf("%d ", gathered_matrix[i * nx + j]);
        }
        printf("\n");
    }
    printf("\n");

    // Allocate memory for the reordered matrix
    short int *reordered_matrix = malloc(ny * nx * sizeof(short int));

    // Iterate over each block (from rank 1 to rank size - 1)
    for (int i = 0; i < size; i++) {
        // Compute the starting index of the block in the gathered matrix
        int block_start_index = i * my_rows * nx;

        // Compute the starting index of the block in the reordered matrix
        int reorder_start_index = i * nx; // i don't know the order in which the gathereed matrix is written!

        // Copy each row of the block to the corresponding position in the reordered matrix
        for (int j = 0; j < my_rows; j++) {
            memcpy(&reordered_matrix[reorder_start_index + j * size * nx], &gathered_matrix[block_start_index + j * nx], nx * sizeof(short int));
        }
    }

    // Output the reordered matrix
    printf("Reordered Matrix:\n");
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            printf("%d ", reordered_matrix[i * nx + j]);
        }
        printf("\n");
    }
    printf("\n");

    // Free memory
    free(reordered_matrix);
}






MPI_Finalize();

return 0;
}

