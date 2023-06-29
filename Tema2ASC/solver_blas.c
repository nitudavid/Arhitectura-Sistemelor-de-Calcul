/*
 * Tema 2 ASC
 * 2023 Spring
 */
#include "utils.h"
#include <cblas.h>

/* 
 * Add your BLAS implementation here
 */
double* transpose(int N, double *M) {
	double *Mt = malloc(N * N * sizeof(double));
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			Mt[N * i + j] = 0;
		}
	}
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			Mt[N * i + j] = M[N * j + i];
		}
	}
	return Mt;
}

double* my_solver(int N, double *A, double *B) {
	printf("BLAS SOLVER\n");
    double *C = malloc(N * N * sizeof(double));
    double *At = malloc(N * N * sizeof(double));
    double *Bt = malloc(N * N * sizeof(double));

    At = transpose(A);
    Bt = transpose(B);

    cblas_dtrmm(CblasRowMajor, CblasLeft, CblasLower, CblasTrans, CblasNonUnit, N, N, 1.0, A, N, B, N);
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, N, N, N, 1.0, B, N, At, N, 1.0, C, N);
    cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans, N, N, N, 1.0, Bt, N, B, N, 1.0, C, N);

    return C;

}


// int main() {
//     int N = 3; // matrix size
//     double alpha = 1.0, beta = 1.0; // coefficients

//     // allocate memory for matrices A, B, At and Bt
//     double *A = (double*)malloc(N*N*sizeof(double));
//     double *B = (double*)malloc(N*N*sizeof(double));

//     A[0] = 1;
// 	A[1] = 2;
// 	A[2] = 3;
// 	A[3] = 0;
// 	A[4] = 1;
// 	A[5] = 2;
// 	A[6] = 0;
// 	A[7] = 0;
// 	A[8] = 1;
// 	B[0] = 1;
// 	B[1] = 2;
// 	B[2] = 3;
// 	B[3] = 4;
// 	B[4] = 5;
// 	B[5] = 6;
// 	B[6] = 7;
// 	B[7] = 8;
// 	B[8] = 9;
//     // compute At and Bt using BLAS functions
//     cblas_dcopy(N*N, A, 1, At, 1);
//     cblas_dcopy(N*N, B, 1, Bt, 1);
//     cblas_dtrmm(CblasRowMajor, CblasLeft, CblasUpper, CblasTrans, CblasNonUnit, N, N, alpha, A, N, At, N);
//     cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, alpha, B, N, At, N, beta, Bt, N);
//     cblas_dsyrk(CblasRowMajor, CblasUpper, CblasTrans, N, N, beta, B, N, alpha, Bt, N);

//     // allocate memory for matrix C
//     double *C = (double*)malloc(N*N*sizeof(double));

//     // compute C using BLAS functions
//     cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, N, N, alpha, Bt, N, Bt, N, beta, C, N);

//     // print matrix C
//     printf("Matrix C:\n");
//     for (int i=0; i<N; i++) {
//         for (int j=0; j<N; j++) {
//             printf("%f ", C[i*N+j]);
//         }
//         printf("\n");
//     }

//     // free memory
//     free(A);
//     free(B);
//     free(At);
//     free(Bt);
//     free(C);

//     return 0;
// }

