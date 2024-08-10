#include <stdio.h>
#include <stdlib.h>
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
    short iter = 0;
    while (zr * zr + zi * zi < 4.0 && iter < max_iterations) {
        f_c(&zr, &zi, cr, ci); // Update zr and zi using the Mandelbrot equation
        iter++;
    }
    return iter; // Return the number of iterations
}

int main(int argc, char *argv[]) {

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
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Number of processes

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
            printf("If you want to set your own parameters you have to run the executable with these arguments: n_x  n_y  x_L  y_L  x_R  y_R  I_max. \n");
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

    int num_threads = omp_get_num_threads();
    printf("Number of threads: %d\n", num_threads);
    int my_rows = ny / size;
    int remainder = ny % size;
    int my_remainder = 0;
    if (rank < remainder) {
        int my_remainder = 1;
    }

    printf("Rank %d num rows: %d \n", rank, my_rows);

    // Allocate memory for local matrix
    char *local_matrix = (char *) malloc(my_rows * nx * sizeof(char));

    // Barrier to synchronize all processes and measure the elapsed time, to check that the workload is balanced
    MPI_Barrier(MPI_COMM_WORLD);

    // Compute Mandelbrot set for local rows with OpenMP parallelization
    #pragma omp parallel for schedule(dynamic) if(num_threads > 1)
    for (int j = 0; j < my_rows + my_remainder; j++) {
        for (int i = 0; i < nx; i++) {
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            double ci = y_L + (y_R - y_L) * (j * size + rank) / (ny - 1);
            local_matrix[j * nx + i] = mandelbrot(cr, ci, I_max);
        }
    }

    end_time = MPI_Wtime();
    printf("Elapsed time for rank %d: %f\n", rank, end_time - start_time);

    char *gathered_matrix = NULL;

    if (size > 1) {
        // Allocate memory for the gathered matrix on rank 0
        if (rank == 0) {
            gathered_matrix = (char *) malloc(ny * nx * sizeof(char));
        }
        // Allocate memory for row_buffer
        char *row_buffer = (char *) malloc(nx * sizeof(char));

        for (int j = 0; j < my_rows; j++) {
            // Copy a row from local_matrix to row_buffer
            memcpy(row_buffer, local_matrix + j * nx, nx * sizeof(char));
            // Gather the row from each process into the gathered_matrix
            MPI_Gather(row_buffer, nx, MPI_CHAR,
                    gathered_matrix + j * size * nx, nx, MPI_CHAR,
                    0, MPI_COMM_WORLD); 
        }

        if (my_remainder == 1) {
            memcpy(row_buffer, local_matrix + my_rows * nx, nx * sizeof(char));
            MPI_Gather(row_buffer, nx, MPI_CHAR,
                    gathered_matrix + my_rows * size * nx, nx, MPI_CHAR,
                    0, MPI_COMM_WORLD);
        }

        free(row_buffer);
        free(local_matrix);
    }

    if (rank == 0) {
        FILE *file = fopen("figures/mandelbrot.pgm", "wb");
        // set the number of different grey levels to 50
        fprintf(file, "P5\n%d %d\n50\n", nx, ny);
        if (size == 1) {
            fwrite(local_matrix, sizeof(char), nx * ny, file);
            free(local_matrix);
        } else {
            fwrite(gathered_matrix, sizeof(char), nx * ny, file);
            free(gathered_matrix);
        }
        end_time = MPI_Wtime();
        printf("Elapsed time: %f\n", end_time - start_time);
        fclose(file);
        printf("Image written\n");
    }
    
    MPI_Finalize();
    return 0;
}