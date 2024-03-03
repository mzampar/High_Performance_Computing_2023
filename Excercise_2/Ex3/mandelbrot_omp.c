#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

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
    if (argc != 8) {
        printf("Usage: %s nx ny x_L y_L x_R y_R I_max\n", argv[0]);
        exit(1);
    }

    int nx = atoi(argv[1]);  // Number of pixels in x-direction
    int ny = atoi(argv[2]);  // Number of pixels in y-direction
    double x_L = atof(argv[3]);  // Left bound of the complex plane
    double y_L = atof(argv[4]);  // Lower bound of the complex plane
    double x_R = atof(argv[5]);  // Right bound of the complex plane
    double y_R = atof(argv[6]);  // Upper bound of the complex plane
    int I_max = atoi(argv[7]);  // Maximum number of iterations



    // Allocate memory for the matrix of short ints
    short int *matrix = malloc(ny * nx * sizeof(short int));
 
    // OpenMP parallel region to compute Mandelbrot set
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            // Calculate complex point corresponding to current pixel
            double cr = x_L + (x_R - x_L) * i / (nx - 1);
            double ci = y_L + (y_R - y_L) * j / (ny - 1);

            // Compute Mandelbrot iteration for the current point
            int iter = mandelbrot(cr, ci, I_max);

            matrix[j * nx + i] = (iter < I_max) ? iter : 0;
        }
    }


    FILE* pgmimg; 
    pgmimg = fopen("figures/mandelbrot.pgm", "wb"); 
  
    // Writing Magic Number to the File 
    fprintf(pgmimg, "P2\n");  
  
    // Writing Width and Height 
    fprintf(pgmimg, "%d %d\n", nx, ny);  
  
    // Writing the maximum gray value 
    fprintf(pgmimg, "90\n");  
    short int temp = 0; 
    for (int i = 0; i < ny; i++) { 
        for (int j = 0; j < nx; j++) { 
            fprintf(pgmimg, "%d ", matrix[i*nx + j]); 
        } 
        fprintf(pgmimg, "\n"); 
    } 
    fclose(pgmimg);
    printf("Image written \n");



    // Free allocated memory
    free(matrix);

    return 0;
}