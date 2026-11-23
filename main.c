#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define NUM_RUNS 20

void dot_product_c(int n, double* A, double* B, double* sdot) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += A[i] * B[i];
    }
    *sdot = sum;
}

int main() {
    int exponents[] = { 20, 24, 28 };
    int num_sizes = sizeof(exponents) / sizeof(exponents[0]);

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    srand((unsigned int)time(NULL));

    for (int s = 0; s < num_sizes; s++) {
        int exp = exponents[s];
        int n = 1 << exp;

        printf("====================================================\n");
        printf("Testing Vector Size n = 2^%d (%d elements)\n", exp, n);
        printf("====================================================\n");

        double* A = (double*)malloc((size_t)n * sizeof(double));
        double* B = (double*)malloc((size_t)n * sizeof(double));

        if (!A || !B) {
            printf("Memory allocation failed for n = 2^%d.\n", exp);
            free(A);
            free(B);
            continue;
        }

        for (int i = 0; i < n; i++) {
            A[i] = (double)rand() / (double)RAND_MAX;
            B[i] = (double)rand() / (double)RAND_MAX;
        }

        double sdot_c = 0.0;
        double total_time_c = 0.0;

        for (int r = 0; r < NUM_RUNS; r++) {
            QueryPerformanceCounter(&start);
            dot_product_c(n, A, B, &sdot_c);
            QueryPerformanceCounter(&end);
            total_time_c += (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        double avg_time_c = total_time_c / NUM_RUNS;

        printf("[C Kernel] sdot = %.6f | Avg Time (%d runs): %.4f ms\n\n", sdot_c, NUM_RUNS, avg_time_c);

        free(A);
        free(B);
    }

    return 0;
}