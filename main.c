#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#define NUM_RUNS 20

#ifdef _DEBUG
#define BUILD_MODE "DEBUG"
#else
#define BUILD_MODE "RELEASE"
#endif

extern void dot_product_asm(int n, double* A, double* B, double* sdot);


__declspec(noinline)    //noinline so the compiler can't fold repeated calls in release mode
void dot_product_c(int n, double* A, double* B, double* sdot) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += A[i] * B[i];
    }
    *sdot = sum;
}

int main() {
    int exponents[] = { 20, 24, 30 };    //spec says 2^20, 2^24, 2^30. If 2^30 can't be allocated, fall back to 2^29 then 2^28
    int num_sizes = sizeof(exponents) / sizeof(exponents[0]);
    const int MIN_EXP_FALLBACK = 28;

    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);

    srand((unsigned int)time(NULL));

    printf("Build mode: %s\n\n", BUILD_MODE);

    for (int s = 0; s < num_sizes; s++) {
        int exp = exponents[s];
        double* A = NULL;
        double* B = NULL;

        /* try the requested size, reduce only if memory is not available */
        while (1) {
            size_t bytes = ((size_t)1 << exp) * sizeof(double);
            A = (double*)malloc(bytes);
            B = (double*)malloc(bytes);
            if (A && B) break;
            free(A);
            free(B);
            A = B = NULL;
            printf("Allocation failed for n = 2^%d.\n", exp);
            if (exp > MIN_EXP_FALLBACK) {
                exp--;
                printf("Reducing to n = 2^%d.\n", exp);
            }
            else {
                break;
            }
        }
        if (!A || !B) {
            printf("Skipping this size.\n\n");
            continue;
        }

        int n = 1 << exp;

        printf("====================================================\n");
        printf("[%s] Vector Size n = 2^%d (%d elements)\n", BUILD_MODE, exp, n);
        printf("====================================================\n");

        for (int i = 0; i < n; i++) {
            A[i] = (double)rand() / (double)RAND_MAX;
            B[i] = (double)rand() / (double)RAND_MAX;
        }

        double sdot_c = 0.0;
        double sdot_asm = 0.0;
        double total_time_c = 0.0;
        double total_time_asm = 0.0;

        for (int r = 0; r < NUM_RUNS; r++) {
            QueryPerformanceCounter(&start);
            dot_product_c(n, A, B, &sdot_c);
            QueryPerformanceCounter(&end);
            total_time_c += (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        double avg_time_c = total_time_c / NUM_RUNS;

        for (int r = 0; r < NUM_RUNS; r++) {
            QueryPerformanceCounter(&start);
            dot_product_asm(n, A, B, &sdot_asm);
            QueryPerformanceCounter(&end);
            total_time_asm += (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
        }
        double avg_time_asm = total_time_asm / NUM_RUNS;

        printf("[C Kernel]      sdot = %.6f | Avg Time (%d runs): %.4f ms\n", sdot_c, NUM_RUNS, avg_time_c);
        printf("[x86-64 Kernel] sdot = %.6f | Avg Time (%d runs): %.4f ms\n", sdot_asm, NUM_RUNS, avg_time_asm);
        printf("Speedup (C time / x86-64 time): %.3fx\n", avg_time_c / avg_time_asm);

        double diff = fabs(sdot_c - sdot_asm);
        double tolerance = 1e-5 * fabs(sdot_c);
        if (diff <= tolerance) {
            printf("Sanity Check:   PASSED (x86-64 kernel output is correct)\n\n");
        }
        else {
            printf("Sanity Check:   FAILED (Difference: %.10f)\n\n", diff);
        }

        free(A);
        free(B);
    }

    return 0;
}
