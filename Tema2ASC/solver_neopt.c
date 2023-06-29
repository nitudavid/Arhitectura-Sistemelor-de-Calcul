/*
 * Tema 2 ASC
 * 2023 Spring
 */
#include "utils.h"

/*
 * Add your unoptimized implementation here
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

double* my_solver(int N, double *A, double* B) {
	printf("NEOPT SOLVER\n");
	
	double *C = malloc(N * N * sizeof(double));

	double *T1 = malloc(N * N * sizeof(double));
	double *T2 = malloc(N * N * sizeof(double));
	double *B2 = malloc(N * N * sizeof(double));

	double *At = malloc(N * N * sizeof(double));
	double *Bt = malloc(N * N * sizeof(double));
	At = transpose(N, A);
	Bt = transpose(N, B);

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			C[N * i + j] = 0;
			T1[N * i + j] = 0;
			T2[N * i + j] = 0;
			B2[N * i + j] = 0;
		}
	}

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			for(int k = i; k < N; k++){
				T1[N * i + j] += A[N * i + k] * B[N * k + j];
			}
		}
	}

	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			for(int k = j; k < N; k++) {
				T2[N * i + j] += T1[N * i + k] * At[N * k + j];
			}
		}
	}
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			for(int k = 0; k < N; k++){
				B2[N * i + j] += Bt[N * i + k] * Bt[N * k + j];
			}
		}
	}
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			C[N * i + j] = T2[N * i + j] + B2[N * i + j];
		}
	}
	
	return C;
}

// int main(){
// 	int N = 3;
// 	double *A = malloc(N * N * sizeof(double));
// 	double *B = malloc(N * N * sizeof(double));
// 	double *C = malloc(N * N * sizeof(double));

// 	double *T1 = malloc(N * N * sizeof(double));
// 	double *T2 = malloc(N * N * sizeof(double));
// 	double *B2 = malloc(N * N * sizeof(double));

// 	double *At = malloc(N * N * sizeof(double));
// 	double *Bt = malloc(N * N * sizeof(double));
// 	Bt = transpose(N, B);
// 	for(int i = 0; i < N; i++) {
// 		for(int j = 0; j < N; j++) {
// 			C[N * i + j] = 0;
// 			T1[N * i + j] = 0;
// 			T2[N * i + j] = 0;
// 			B2[N * i + j] = 0;
// 			At[N * i + j] = 0;
// 		}
// 	}
	

// 	A[0] = 1;
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

// 	At = transpose(N, A);

// 	for(int i = 0; i < N; i++) {
// 		for(int j = 0; j < N; j++) {
// 			for(int k = i; k < N; k++){
// 				T1[N * i + j] += A[N * i + k] * B[N * k + j];
// 			}
// 		}
// 	}

// 	for(int i = 0; i < N; i++) {
// 		for(int j = 0; j < N; j++) {
// 			for(int k = j; k < N; k++) {
// 				T2[N * i + j] += T1[N * i + k] * At[N * k + j];
// 			}
// 		}
// 	}

// 	C = my_solver(N, A, B);

// 	for(int i = 0; i < N; i++) {
// 		for(int j = 0; j < N; j++) {
// 			printf("%f ", C[N * i + j]);
// 		}
// 	}
// 	// printf("\n");
// 	// for(int i = 0; i < N; i++) {
// 	// 	for(int j = 0; j < N; j++) {
// 	// 		printf("%f ", T1[N * i + j]);
// 	// 	}
// 	// }
// 	// printf("\n");
// 	// for(int i = 0; i < N; i++) {
// 	// 	for(int j = 0; j < N; j++) {
// 	// 		printf("%f ", At[N * i + j]);
// 	// 	}
// 	// }
// 	// printf("\n");
// 	// for(int i = 0; i < N; i++) {
// 	// 	for(int j = 0; j < N; j++) {
// 	// 		printf("%f ", T2[N * i + j]);
// 	// 	}
// 	// }
// 	printf("\n");
// }
