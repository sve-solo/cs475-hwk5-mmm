#ifndef MMM_H_
#define MMM_H_

// globals (anything here would be shared with all threads) */
// I would declare the pointers to the matrices here (i.e., extern double **A, **B, **C),
// as well as the size of the matrices, etc.

extern double **matrixA; //input matrix
extern double **matrixB; //input matrix
extern double **matrixC; //output matrix
extern double **partialMatrix; //partial output matrix 

extern double **parCopy; //copy of parallel matrix output

extern int size; //size of matrix (size by size)


void mmm_init();
void mmm_reset(double **);
void mmm_freeup();
void mmm_seq();
void *mmm_par(void *args);
double mmm_verify();

typedef struct thread_args{
	//thread id
	int tid;

	//starting position
	int start;

	//ending position
	int end;
}thread_args;
#endif /* MMM_H_ */
