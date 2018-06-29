/*!
 * \page p2p_eager Eager protocol point-to-point benchmark
 *
 * Point-to-point benchmarks using MPI_Rsend/MPI_Recv pairs. It usually performs better
 * than MPI_Send/MPI_Recv on high-latency links, probably because it uses the eager protocol.
 *
 * The maximum time measured at sender or receiver is taken, and after a number of iterations the minimum
 * time is taken.
 */

#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define SIZE 1000000
#define MAX_ITER 16
#define MAX_STEPS 32

int main(int argc, char **argv) {

        char * array;
        double start[MAX_ITER], finish[MAX_ITER];
        MPI_Comm comm;
        int rank, size;
	int j,k;
	double totalTime=0.0;
	double minTime=1000.;

        MPI_Init(&argc, &argv);
        comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	array = (char *) malloc(SIZE * sizeof(char));
	
	if (rank == 0) {
		double time[MAX_ITER];
		double finalTime[MAX_ITER];
		for (k=0;k < MAX_STEPS; k++) {
			for (j=1; j < MAX_ITER; j++) {
				MPI_Barrier(comm);
				start[j] = MPI_Wtime();
				MPI_Rsend(array, (SIZE*(k+1))/MAX_STEPS, MPI_CHAR, 1, 0, comm);
				finish[j] = MPI_Wtime();
				time[j] = finish[j] - start[j];
				MPI_Reduce(&time[j], &finalTime[j], 1, MPI_DOUBLE, MPI_MAX, 0, comm);
			}
			totalTime=0.0;
			minTime = 100000.;
			for (j=1; j<MAX_ITER; j++) {
				totalTime+=finalTime[j];
				minTime= (finalTime[j] < minTime)?finalTime[j]:minTime;
			}
			//printf("%d\t%lf\n",(SIZE*(k+1))/(MAX_STEPS), totalTime/(double)MAX_ITER);
			printf("%d\t%lf\n",(SIZE*(k+1))/(MAX_STEPS), minTime);
			
			
				
		}
	}

	if (rank == 1) {
		double time[MAX_ITER];
		double finalTime[MAX_ITER];
		for (k=0;k < MAX_STEPS; k++) {
			for (j=1; j < MAX_ITER; j++) {
				MPI_Barrier(comm);
				start[j] = MPI_Wtime();
				MPI_Recv(array, (SIZE*(k+1))/MAX_STEPS, MPI_CHAR, 0, 0, comm, MPI_STATUS_IGNORE);
				finish[j] = MPI_Wtime();
				time[j] = finish[j] - start[j];
				MPI_Reduce(&time[j], &finalTime[j], 1, MPI_DOUBLE, MPI_MAX, 0, comm);
			}
		}
	}

	if (rank == 0) {
	}
        MPI_Finalize();
        return 0;
}

