#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
    if (argc != 8) {
        printf("Usage: %s nx ny x_L y_L x_R y_R I_max\n", argv[0]);
        exit(1);
    }

    int provided_thread_level;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided_thread_level);
    if (provided_thread_level < MPI_THREAD_FUNNELED) {
        printf("Error: Could not initialize MPI with required threading level.\n");
        MPI_Finalize();
        exit(1);
    }

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

    int rows_per_process = ny / size;
    int remainder = ny % size;

    // Calculate start and end rows for this process
    int start_row = rank * rows_per_process;
    int end_row = start_row + rows_per_process;
    if (rank == size - 1) {
        end_row += remainder;  // Add remainder to the last process
    }

    // Allocate memory for the rows to compute
    int *rows = (int *)malloc((end_row - start_row) * nx * sizeof(int));

    // Compute Mandelbrot set for assigned rows using OpenMP
    #pragma omp parallel for schedule(dynamic)
    for (int j = start_row; j < end_row; j++) {
        double ci = y_L + (y_R - y_L) * j / (ny - 1);
        for (int i = 0; i < nx; i++) {
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            int iter = mandelbrot(cr, ci, I_max);
            rows[(j - start_row) * nx + i] = (iter < I_max) ? iter : 0;
        }
    }

    // Gather computed rows at root process
    int *recvcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        recvcounts[i] = rows_per_process * nx;
        displs[i] = i * rows_per_process * nx;
    }
    if (rank == size - 1) {
        recvcounts[size - 1] += remainder * nx;  // Adjust for remainder
    }

    int *matrix = NULL;
    if (rank == 0) {
        matrix = (int *)malloc(ny * nx * sizeof(int));
    }
    MPI_Gatherv(rows, (end_row - start_row) * nx, MPI_INT, matrix, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

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
