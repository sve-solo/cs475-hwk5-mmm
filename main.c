#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

/*
* Global defs
*/
int size;
double **parCopy;

int main(int argc, char *argv[]) {
	double clockstart, clockend;

	//sequential mode
	//handle if user inputs incorrect args
	if(argc == 1){
		printf("Usage: ./mmm <mode> [num threads] <size>\n");
	}
	else if(strcmp(argv[1], "S") == 0 && argc == 3){
		//array to store sequential time runs
		double seqRuns[4];

		//int size;
		sscanf(argv[2], "%d", &size);
		//printf("size: %d\n", size);
		if(size <= 0){
			printf("Error: Enter a positive size.\n");
			return 0;
		}

		//initialize matrices + gets space on heap for matrices
		mmm_init();

		//run program 4x to smooth results
		for(int i = 0; i < 4; i++){
			//start clocking sequential
			clockstart = rtclock();

			//do sequential multiplication
			mmm_seq();

			//stop clocking
			clockend = rtclock(); 

			//free up space
			//mmm_freeup();

			//get sequential clock time
			double seqTime = (clockend - clockstart);
			seqRuns[i] = seqTime;
			//printf("Sequential time taken: %.6f sec\n", seqTime);

		}

		//free up space
		mmm_freeup();

		//add up time from runs 1 - 4 and divide by 3
		double seqTotal = 0.0;
		for(int i = 1; i < 4; i++){
			seqTotal = seqTotal + seqRuns[i];
			//printf("seqRuns[i]: %f\n", seqRuns[i]);
		}
		//printf("total: %f\n", seqTotal);
		seqTotal = seqTotal / 3.0;
		//printf("div total: %f\n", seqTotal);
		printf("========\n");
		printf("mode: sequential\n");
		printf("thread count: 1\n");
		printf("size: %d\n", size);
		printf("========\n");
		printf("Sequential Time (avg of 3 runs): %.6f sec\n", seqTotal);
		
	}
	//parallel mode
	else if(strcmp(argv[1], "P") == 0 && argc == 4){
		//array to store parallel time runs
		double parRuns[4];

		//array to store sequential time runs
		double seqRuns[4];

		//int size;
		size = 0;
		sscanf(argv[3], "%d", &size);
		//printf("size: %d\n", size);
		if(size <= 0){
			printf("Error: Enter a positive size.\n");
			return 0;
		}

		int num_threads;
		sscanf(argv[2], "%d", &num_threads);
		//printf("num_threads: %d\n", num_threads);
		if(num_threads <= 0 || num_threads > size){
			printf("Error.\n");
			return 0;
		
		}

		//allocate array to hold copy of parallel output
    	parCopy = (double**) malloc(sizeof(double*) * size);
	
		//iterate through each row and malloc matrices of doubles
		for(int i = 0; i < size; i++){
			parCopy[i] = (double*) malloc(sizeof(double) * size);
		}

		//initialize matrices for parallel mode
		mmm_init();

		//run parallel program 4x to smooth results
		for(int i = 0; i < 4; i++){
			//start clocking parallel mode
			clockstart = rtclock();

			//malloc space to hold partial output matrix
			partialMatrix = (double**) malloc(sizeof(double*) * size);

			//iterate through each row and malloc matrices of doubles
			for(int i = 0; i < size; i++){
				partialMatrix[i] = (double*) malloc(sizeof(double) * size);
			}

			//set up thread arguments
  			thread_args *args = (thread_args*) malloc(sizeof(thread_args) * num_threads);
  			for (int i = 0; i < num_threads; i++) {
    			args[i].tid = i;
    			args[i].start = i * size / num_threads + 1;
    			args[i].end = (i + 1) * size / num_threads;
				//printf("tid: %d, start: %d, end: %d\n", args[i].tid, args[i].start, args[i].end);
  			}

			//allocate space to hold threads
			pthread_t *threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));

			//printf("numthreads: %d", num_threads);
			//create threads
			for(int i = 0; i < num_threads; i++){
				pthread_create(&threads[i], NULL, mmm_par, &args[i]);
			}

			//join threads
			for(int i = 0; i < num_threads; i++){
				pthread_join(threads[i], NULL);
			}

			//printf("In main:\n");
			//move partial matrix results into output matrix
			for(int i = 0; i < size; i++){
				for(int j = 0; j < size; j++){
					//printf("partial[i][j]: %f  ", partialMatrix[i][j]);
					//printf("matrixC[i][j]: %f\n", matrixC[i][j]);
					matrixC[i][j] = partialMatrix[i][j];
				
				}
				//printf("\n");
			}

			//free each row in matrix arrays
			for(int i = 0; i < size; i++){
    			free(partialMatrix[i]);
    			partialMatrix[i] = NULL;
			}

			//free partial matrix
			free(partialMatrix);
			partialMatrix = NULL;

			//free thread arguments
			free(args);
			args = NULL;

			//free threads
			free(threads);
			threads = NULL;

			//stop clocking
			clockend = rtclock(); 

			//get parallel clock time
			double parTime = (clockend - clockstart);
			parRuns[i] = parTime;
			//printf("Parallel time taken: %.6f sec\n", parTime);

			//move parallel output from matrixC into parCopy
			for(int i = 0; i < size; i++){
				for(int j = 0; j < size; j++){
					parCopy[i][j] = matrixC[i][j];
				}
			}

		}

		//set matrixC to zeros... necessary bc you already free?
		mmm_reset(matrixC);

		//run sequential mode 4x
		for(int i = 0; i < 4; i++){

			//start clocking sequential
			clockstart = rtclock();

			//do sequential multiplication
			mmm_seq();

			//stop clocking
			clockend = rtclock(); 

			//time it
			double seqTime = (clockend - clockstart);
			seqRuns[i] = seqTime;
			//printf("Sequantial time taken: %.6f sec\n", seqTime);

		}

		//calculate parallel time
		//add up time from runs 1 - 4 and divide by 3
		double parTotal;
		for(int i = 1; i < 4; i++){
			parTotal = parTotal + parRuns[i];
			//printf("seqRuns[i]: %f\n", seqRuns[i]);
		}
		//printf("total: %f\n", total);
		parTotal = parTotal / 3.0;

		//calculate sequential time
		//add up time from runs 1 - 4 and divide by 3
		double seqTotal;
		for(int i = 1; i < 4; i++){
			seqTotal = seqTotal + seqRuns[i];
			//printf("seqRuns[i]: %f\n", seqRuns[i]);
		}
		//printf("total: %f\n", total);
		seqTotal = seqTotal / 3.0;

		//printf("div total: %f\n", total);
		//check that total is correct
		printf("========\n");
		printf("mode: parallel\n");
		printf("thread count: %d\n", num_threads);
		printf("size: %d\n", size);
		printf("========\n");
		printf("Sequential Time (avg of 3 runs): %.6f sec\n", seqTotal);
		printf("Parallel Time (avg of 3 runs): %.6f sec\n", parTotal);  
		printf("Speedup: %f\n", (seqTotal / parTotal));

		//make copy of sequential output, so you can later compare them

		//check that parallel mode matches sequential mode
		double result = mmm_verify();
		printf("Verifying... largest error between parallel and sequential matrix: %f\n", result);

		//free up space for sequential mode
		mmm_freeup();

		//free parCopy
		for(int i = 0; i < size; i++){
    		free(parCopy[i]);
    		parCopy[i] = NULL;
 		}

		free(parCopy);
		parCopy = NULL;
	}
	//handle if user inputs incorrect args
	else{
		printf("Usage: ./mmm <mode> [num threads] <size>\n");
	}
	
	return 0;
}
