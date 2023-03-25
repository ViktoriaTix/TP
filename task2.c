#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define Matrix 1024
#define accuracy 1e-6
#define iterations 1000000

int main() {
    // Allocate 2D arrays on host memory
    double* arr = (double*)malloc(Matrix * Matrix * sizeof(double));
    double* array_new = (double*)malloc(Matrix * Matrix * sizeof(double));
    // Initialize arrays to zero
    for (int i = 0; i < Matrix * Matrix; i++) {
        arr[i] = 0;
        array_new[i] = 0;
    }
    // Set boundary conditions
    arr[0 * Matrix + 0] = 10;
    arr[0 * Matrix + Matrix - 1] = 20;
    arr[(Matrix - 1) * Matrix + 0] = 30;
    arr[(Matrix - 1) * Matrix + Matrix - 1] = 20;

    // Main loop
    double err = accuracy + 1;
    int iter = 0;

#pragma acc data copy(arr[0:Matrix*Matrix], arrNew[0:Matrix*Matrix])
    {
#pragma acc kernels loop independent
        for (int j = 1; j < Matrix; j++) {
            arr[0 * Matrix + j] = (arr[0 * Matrix + Matrix - 1] - arr[0 * Matrix + 0]) / (Matrix - 1) + arr[0 * Matrix + j - 1];   //top
            arr[(Matrix - 1) * Matrix + j] = (arr[(Matrix - 1) * Matrix + Matrix - 1] - arr[(Matrix - 1) * Matrix + 0]) / (Matrix - 1) + arr[(Matrix - 1) * Matrix + j - 1]; //bottom
            arr[j * Matrix + 0] = (arr[(Matrix - 1) * Matrix + 0] - arr[0 * Matrix + 0]) / (Matrix - 1) + arr[(j - 1) * Matrix + 0]; //left
            arr[j * Matrix + Matrix - 1] = (arr[(Matrix - 1) * Matrix + Matrix - 1] - arr[0 * Matrix + Matrix - 1]) / (Matrix - 1) + arr[(j - 1) * Matrix + Matrix - 1]; //right
        }


        while (err > accuracy && iter < iterations) {
            // Compute new values
            err = 0;
#pragma acc kernels loop independent reduction(max:err)
            for (int j = 1; j < Matrix - 1; j++) {
#pragma acc loop reduction(max:err)
                for (int i = 1; i < Matrix - 1; i++) {
                    int index = j * Matrix + i;
                    array_new[index] = 0.25 * (arr[index + Matrix] + arr[index - Matrix] +
                        arr[index - 1] + arr[index + 1]);
                    err = fmax(err, fabs(array_new[index] - arr[index]));
                }
            }
            // Update values
#pragma acc kernels loop independent
            for (int j = 1; j < Matrix - 1; j++) {
#pragma acc loop
                for (int i = 1; i < Matrix - 1; i++) {
                    int index = j * Matrix + i;
                    arr[index] = array_new[index];
                }
            }

            iter++;

            // Print progress
            if (iter % 100 == 0) {
                printf("%d, %0.6lf\n", iter, err);
            }
        }
    }

    printf("Final result: %d, %0.6lf\n", iter, err);
    // Free memory
    free(arr);
    free(array_new);

    return 0;
}
