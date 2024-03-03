Compilation:

gcc -o main mandelbrot_omp.c -lm -fopenmp

Execution:

./main 10000 10000 -2.25 -1.5 0.75 1.5 500

gcc -o main mandelbrot_half_omp.c -lm -fopenmp
./main_half 10000 10000 -2.25 -1.5 0.75 1.5 500