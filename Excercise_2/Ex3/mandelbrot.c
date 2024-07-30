#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include<omp.h>
#include<string.h>

// Function to update the complex values zr and zi using the Mandelbrot equation
void f_c(double *zr, double *zi, double cr, double ci) {
    double zr_new = (*zr) * (*zr) - (*zi) * (*zi) + cr;
    double zi_new = 2 * (*zr) * (*zi) + ci;
    *zr = zr_new;
    *zi = zi_new;
}

// Function to check if a complex point belongs to the Mandelbrot set
int mandelbrot(double cr, double ci, int max_iterations) {
    double zr = 0.0, zi = 0.0;
    int iter = 0;
    while (zr * zr + zi * zi < 4.0 && iter < max_iterations) {
        f_c(&zr, &zi, cr, ci); // Update zr and zi using the Mandelbrot equation
        iter++;
    }
    return iter;
}

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

    double start_time, end_time;
    start_time = MPI_Wtime();

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
   // int my_rows = (rank < remainder) ? rows_per_process + 1 : rows_per_process;
    //my_rows += (rank < remainder ? 1 : 0);

    int my_rows;

    if (rank < remainder) {
        my_rows = rows_per_process + 1;
    } else {
        my_rows = rows_per_process;
    }

    printf("my_rows: %d of rank %d \n ", my_rows, rank);

    // Allocate memory for storing the number of rows for each process
    int *rows_per_process_array = NULL;
    if (rank == 0) {
        rows_per_process_array = malloc(size * sizeof(int));
    }

    // Gather the number of rows for each process to rank 0
    MPI_Gather(&my_rows, 1, MPI_INT, rows_per_process_array, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Output the number of rows for each process (only on rank 0)
    if (rank == 0) {
        printf("Number of Rows for Each Process:\n");
        for (int i = 0; i < size; i++) {
            printf("Process %d: %d rows\n", i, rows_per_process_array[i]);
        }
        printf("\n");
    }

    // Allocate memory for local matrix
    short int *local_matrix = (short int *) malloc(my_rows * nx * sizeof(short int));

    // Compute Mandelbrot set for local rows with OpenMP parallelization

    #pragma omp parallel for schedule(dynamic) shared(local_matrix) // the problem is that the points are different, not the way i write in the matrix
    
    
    for (int j = rank; j < ny; j+=size) { // Distribute rows among processes 
        for (int i = 0; i < nx; i++) { 
            // Calculate complex point corresponding to current pixel
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            double ci = y_L + (y_R - y_L) * j / (ny - 1); // the probelm is that i loose the first raw because i multiply for size.

            // Compute Mandelbrot iteration for the current point
            int iter = mandelbrot(cr, ci, I_max);

            local_matrix[(j - rank)/size * nx + i] = (iter < I_max) ? iter : 0;
        }
}




    // Calculate the displacement and receive counts for MPI_Gatherv
    int *recv_counts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        recv_counts = malloc(size * sizeof(int));
        displs = malloc(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            recv_counts[i] = rows_per_process_array[i] * nx; // Number of elements to receive from each process
            displs[i] = i * rows_per_process_array[i] * nx;  // Displacement for each process in the receive buffer
        }
    }

    // Allocate memory for receiving the gathered matrix on rank 0
    short int *gathered_matrix = NULL;
    if (rank == 0) {
        gathered_matrix = (short int *)malloc(ny * nx * sizeof(short int));
    }

    // Gather results to rank 0
    MPI_Gatherv(local_matrix, my_rows * nx, MPI_SHORT, gathered_matrix, recv_counts, displs, MPI_SHORT, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();

    printf("Execution time: %f\n", rank, end_time - start_time);

    // Reorder gathered matrix on rank 0
    if (rank == 0) {
        // Allocate memory for the reordered matrix
        short int *reordered_matrix = (short int *)malloc(ny * nx * sizeof(short int));

        // Iterate over each block (from rank 1 to rank size - 1)
        for (int i = 0; i < size; i++) {
            // Compute the starting index of the block in the gathered matrix
            int block_start_index = i * rows_per_process_array[i] * nx; // this is my_rows of master! not of the others

            // Compute the starting index of the block in the reordered matrix
            int reorder_start_index = i * nx; // i don't know the order in which the gathereed matrix is written!

            // Copy each row of the block to the corresponding position in the reordered matrix
            for (int j = 0; j < rows_per_process_array[i]; j++) {
                memcpy(&reordered_matrix[reorder_start_index + j * size * nx], &gathered_matrix[block_start_index + j * nx], nx * sizeof(short int));
            }
        }


    FILE *pgmimg;
    pgmimg = fopen("figures/mandelbrot.pgm", "wb");

    // Writing Magic Number to the File
    fprintf(pgmimg, "P2\n");

    // Writing Width and Height
    fprintf(pgmimg, "%d %d\n", nx, ny);

    // Writing the maximum gray value
    fprintf(pgmimg, "90\n");
    for (int i = 0; i < ny; i++) {
        for (int j = 0; j < nx; j++) {
            fprintf(pgmimg, "%d ", reordered_matrix[i * nx + j]);
        }
        fprintf(pgmimg, "\n");
    }
    fclose(pgmimg);
    printf("Image written\n");
    

    // Free allocated memory
    // Free allocated memory

        free(gathered_matrix);
        free(recv_counts);
        free(displs);
    }

    free(local_matrix);

    MPI_Finalize();

    return 0;
}
