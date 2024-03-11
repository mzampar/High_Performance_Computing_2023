#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

// Function to check if a complex point belongs to the Mandelbrot set
int mandelbrot(double cr, double ci, int max_iterations) {
    double zr = 0.0, zi = 0.0;
    int iter = 0;
    while (zr * zr + zi * zi < 4.0 && iter < max_iterations) {
        double temp = zr * zr - zi * zi + cr;
        zi = 2.0 * zr * zi + ci;
        zr = temp;
        iter++;
    }
    return iter;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int nx = atoi(argv[1]);  // Number of pixels in x-direction
    int ny = atoi(argv[2]);  // Number of pixels in y-direction
    double x_L = atof(argv[3]);  // Left bound of the complex plane
    double y_L = atof(argv[4]);  // Lower bound of the complex plane
    double x_R = atof(argv[5]);  // Right bound of the complex plane
    double y_R = atof(argv[6]);  // Upper bound of the complex plane
    int I_max = atoi(argv[7]);  // Maximum number of iterations

    // Calculate number of rows for each process
    int rows_per_process = ny / size;
    int remainder = ny % size;

    // Determine starting and ending row for this process
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;
    if (rank == size - 1) {
        end_row += remainder;  // Add remainder to the last process
    }

    // Allocate memory for local computation
    int *local_image = (int *)malloc(nx * (end_row - start_row) * sizeof(int));

    // Compute Mandelbrot set locally using OpenMP parallelization
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int j = start_row; j < end_row; j++) {
        for (int i = 0; i < nx; i++) {
            // Calculate complex point corresponding to current pixel
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            double ci = y_L + (y_R - y_L) * j / (ny - 1);

            // Compute Mandelbrot iteration for the current point
            int iter = mandelbrot(cr, ci, I_max);

            // Store result in local image array
            local_image[(j - start_row) * nx + i] = iter > I_max ? I_max : 0;
        }
    }

    // Gather local images from all processes
    int *global_image = NULL;
    if (rank == 0) {
        global_image = (int *)malloc(nx * ny * sizeof(int));
    }
    MPI_Gather(local_image, nx * (end_row - start_row), MPI_INT,
               global_image, nx * (end_row - start_row), MPI_INT,
               0, MPI_COMM_WORLD);

    // Process 0 reconstructs the final image matrix
    if (rank == 0) {
        // Output the final image matrix
        // Here you can use the global_image array to write to a file or perform further processing
    }

    // Free allocated memory
    free(local_image);
    if (rank == 0) {
        free(global_image);
    }

    MPI_Finalize();

    return 0;
}

    /////////////////////////////////////////

    // Output Mandelbrot set (only root process does this)
    if (rank == 0) {
    FILE* pgmimg; 
    pgmimg = fopen("figures/mandelbrot.pgm", "wb"); 
  
    // Writing Magic Number to the File 
    fprintf(pgmimg, "P2\n");  
  
    // Writing Width and Height 
    fprintf(pgmimg, "%d %d\n", nx, ny);  
  
    // Writing the maximum gray value 
    fprintf(pgmimg, "90\n");
    for (int i = 0; i < ny; i++) { 
        for (int j = 0; j < nx; j++) { 
            fprintf(pgmimg, "%d ", matrix[i*nx + j]); 
        } 
        fprintf(pgmimg, "\n"); 
    } 
    fclose(pgmimg);
    printf("Image written \n");
    }

    // Cleanup
    free(rows);
    free(recvcounts);
    free(displs);
    if (rank == 0) {
        free(matrix);
    }

    MPI_Finalize();

    return 0;
}
