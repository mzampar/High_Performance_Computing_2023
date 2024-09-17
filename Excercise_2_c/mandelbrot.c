#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <string.h>
#include "utils.h"
#define WRITE_PGM_IMAGE 0 // Compile with -DWRITE_IMAGE=1 to write the pgm image


int main(int argc, char *argv[]) {

    // Initialize MPI environment for multi-processes
    int mpi_provided_thread_level;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
    if (mpi_provided_thread_level < MPI_THREAD_FUNNELED) {
        printf("The threading support level is lesser than that demanded\n");
        MPI_Finalize();
        exit(1);
    }

    double start_time, end_time;
    start_time = MPI_Wtime();

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Default parameters
    int nx = 1000;
    int ny = 1000; 
    int I_max = 1000;
    double x_L = -1.5;
    double x_R = 0.5;
    double y_L = -1.25;
    double y_R = 1.25;

    if (argc != 8) {
        if (rank == 0) {
            printf("You are using the default parameters.\n");
            printf("If you want to set your own parameters you have to run the executable with these arguments: \n n_x  n_y  x_L  y_L  x_R  y_R  I_max. \n");
        }
    }

    if (argc == 8) {
            nx = atoi(argv[1]);  // Number of pixels in x-direction
            ny = atoi(argv[2]);  // Number of pixels in y-direction
            x_L = atof(argv[3]);  // Left bound of the complex plane
            y_L = atof(argv[4]);  // Lower bound of the complex plane
            x_R = atof(argv[5]);  // Right bound of the complex plane
            y_R = atof(argv[6]);  // Upper bound of the complex plane
            I_max = atoi(argv[7]);  // Maximum number of iterations
    }

    const int my_rows = ny / size;
    const int remainder = ny % size;
    int my_remainder = 0;
    if (rank < remainder) {
        my_remainder = 1;
    }

    // Allocate memory for local matrix
    char *local_matrix = (char *) malloc((my_rows + my_remainder) * nx * sizeof(char));

    // Barrier to synchronize all processes and measure the elapsed time
    MPI_Barrier(MPI_COMM_WORLD);

    double computation_start_time, computation_end_time, computation_time;
    computation_start_time = MPI_Wtime();

    const double delta_x = (x_R - x_L) / (nx - 1);
    const double delta_y = (y_R - y_L) / (ny - 1);

    #pragma omp parallel
    {
        double cr, ci;
        #pragma omp for schedule(dynamic) // Dynamic scheduling to balance the load, since some rows are more complex than others
        for (int j = 0; j < my_rows + my_remainder; j++) {
            ci = y_L + (j * size + rank) * delta_y;
            for (int i = 0; i < nx; i++) {
                cr = x_L + i * delta_x;
                local_matrix[j * nx + i] = mandelbrot(cr, ci, I_max);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    computation_end_time = MPI_Wtime();
    computation_time = computation_end_time - computation_start_time;

    const char *bin_filename = "./figures/mandelbrot_parallel.bin";
    const char *pgm_filename = "./figures/mandelbrot_parallel.pgm";

    // Open file for writing binary data
    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD, bin_filename, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    MPI_Offset row_offset;

    for (int i = 0; i < my_rows; i++) {
        row_offset =  rank * nx + size * i * nx;
        MPI_File_write_at_all(fh, row_offset, local_matrix + i * nx, nx, MPI_CHAR, MPI_STATUS_IGNORE);
    }

    // Write remainder rows
    if (my_remainder == 1) {
        MPI_Offset remainder_offset = rank * nx + size * my_rows * nx;
        MPI_File_write_at(fh, remainder_offset, local_matrix + (my_rows * nx), nx, MPI_CHAR, MPI_STATUS_IGNORE);
    }

    // Close the file
    MPI_File_close(&fh);

    double write_end_time, write_time;
    write_end_time = MPI_Wtime();
    write_time = write_end_time - computation_end_time;

    // Print the elapsed time
    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Computation time: %f seconds\n", computation_time);
        printf("Write time: %f seconds\n", write_time);
        printf("Elapsed time: %f seconds\n", end_time - start_time);
    }

    if (rank == 0 && WRITE_PGM_IMAGE) {
        convert_bin_to_pgm(bin_filename, pgm_filename, nx, ny);
    }
    
    MPI_Finalize();
    return 0;
}