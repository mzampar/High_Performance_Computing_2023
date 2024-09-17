#include <stdio.h>
#include <stdlib.h>

void convert_bin_to_pgm(const char *bin_filename, const char *pgm_filename, int width, int height) {
    FILE *bin_file = fopen(bin_filename, "rb");
    FILE *pgm_file = fopen(pgm_filename, "wb");

    if (bin_file == NULL) {
        perror("Error opening binary file");
        exit(EXIT_FAILURE);
    }

    if (pgm_file == NULL) {
        perror("Error opening PGM file");
        fclose(bin_file);
        exit(EXIT_FAILURE);
    }

    // Write the PGM header
    fprintf(pgm_file, "P5\n%d %d\n255\n", width, height);

    // Allocate buffer for reading one row at a time
    unsigned char *row_buffer = (unsigned char *)malloc(width);

    if (row_buffer == NULL) {
        perror("Error allocating memory");
        fclose(bin_file);
        fclose(pgm_file);
        exit(EXIT_FAILURE);
    }

    // Process one row at a time
    for (int i = 0; i < height; i++) {
        // Read one row of binary data
        size_t read_size = fread(row_buffer, 1, width, bin_file);
        if (read_size != width) {
            perror("Error reading binary data");
            free(row_buffer);
            fclose(bin_file);
            fclose(pgm_file);
            exit(EXIT_FAILURE);
        }

        // Write the row to the PGM file
        fwrite(row_buffer, 1, width, pgm_file);
    }

    // Clean up
    free(row_buffer);
    fclose(bin_file);
    fclose(pgm_file);
}


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