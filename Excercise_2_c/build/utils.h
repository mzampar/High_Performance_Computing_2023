#ifndef UTILS_H
#define UTILS_H

void convert_bin_to_pgm(const char *bin_filename, const char *pgm_filename, int width, int height);
void f_c(double *zr, double *zi, double cr, double ci);
int mandelbrot(double cr, double ci, int max_iterations);

#endif