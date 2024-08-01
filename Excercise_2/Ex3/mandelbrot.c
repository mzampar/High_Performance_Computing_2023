#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>

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
int main(int argc, unsigned char *argv[]) {
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

    int my_rows;
    if (rank < remainder) {
        my_rows = rows_per_process + 1;
    } else {
        my_rows = rows_per_process;
    }

    printf("my_rows: %d of rank %d \n", my_rows, rank);

    // Allocate memory for storing the number of rows for each process
    int *rows_per_process_array = NULL;
    if (rank == 0) {
        rows_per_process_array = (int *) malloc(size * sizeof(int));
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
    unsigned char *local_matrix = (unsigned char *) malloc(my_rows * nx * sizeof(unsigned char));

    // Compute Mandelbrot set for local rows with OpenMP parallelization
    #pragma omp parallel for schedule(dynamic) shared(local_matrix)
    for (int j = 0; j < my_rows; j++) {
        for (int i = 0; i < nx; i++) {
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            double ci = y_L + (y_R - y_L) * (rank * rows_per_process + j) / (ny - 1);

            int iter = mandelbrot(cr, ci, I_max);
            local_matrix[j * nx + i] = (iter < I_max) ? iter : 0;
        }
    }

    // Calculate the displacement and receive counts for MPI_Gatherv
    int *recv_counts = NULL;
    int *displs = NULL;
    if (rank == 0) {
        recv_counts = (int *) malloc(size * sizeof(int));
        displs = (int *) malloc(size * sizeof(int));
        displs[0] = 0;
        for (int i = 0; i < size; i++) {
            recv_counts[i] = rows_per_process_array[i] * nx;
            if (i > 0) {
                displs[i] = displs[i - 1] + recv_counts[i - 1];
            }
        }
    }

    // Allocate memory for receiving the gathered matrix on rank 0
    unsigned char *gathered_matrix = NULL;
    if (rank == 0) {
        gathered_matrix = (unsigned char *) malloc(ny * nx * sizeof(unsigned char));
    }

    // Gather results to rank 0
    MPI_Gatherv(local_matrix, my_rows * nx, MPI_UNSIGNED_CHAR, gathered_matrix, recv_counts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Elapsed time: %f\n", end_time - start_time);
        FILE *file = fopen("figures/mandelbrot.pgm", "wb");
        fprintf(file, "P5\n%d %d\n255\n", nx, ny);
        fwrite(gathered_matrix, sizeof(unsigned char), nx * ny, file);
        fclose(file);
        free(gathered_matrix);
        free(recv_counts);
        free(displs);
        printf("Image written\n");
    }
    
    MPI_Finalize(); // Finalize the MPI environment
    return 0;
}