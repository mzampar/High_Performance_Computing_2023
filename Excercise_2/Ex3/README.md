Compilation:

gcc -o main mandelbrot_omp.c -lm -fopenmp

Execution:

./main 10000 10000 -2.25 -1.5 0.75 1.5 500

gcc -o main mandelbrot_half_omp.c -lm -fopenmp
./main_half 10000 10000 -2.25 -1.5 0.75 1.5 500



to downaload the image produced in orfeo, from the terminal in the local system:

scp mzampar@195.14.102.215:/u/dssc/mzampar/hpc_project/Excercise_2/Ex3/figures/mandelbrot.pgm /Users/marcozamp/Desktop/orfeo_figures


