/*!
 * \page hybrid_fragment Point-to-point communication by fragmenting in a hybrid MPI/OpenMP approach
 * This standalone program uses a hybrid approach to send a message in fragments
 * from MPI process 0 to MPI process 1.
 * We create a number of OpenMP threads on each MPI process. Each process then
 * uses all OpenMP threads to submit equal fragments of the message. Each fragment
 * is transferred by a different OpenMP thread using MPI_Send/MPI_Recv.
 *
 * The program can only run properly if the MPI library supports MPI_THREAD_MULTIPLE
 *
 * \code
 * $ mpirun -n 2 ./fragment-omp-for
 * \endcode
 *
 */

#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

#define SIZE 1000000
#define MAX_ITER 16
#define MAX_STEPS 16
#define THREAD_NUM 2

int main(int argc, char **argv) {

        char * array;
        double start[MAX_ITER], finish[MAX_ITER];
        MPI_Comm comm;
        int rank, size;
	int i,j,k;
        int provided;
	double totalTime=0.0;
        int required = MPI_THREAD_MULTIPLE;
        char * empty = NULL;
	omp_set_num_threads(THREAD_NUM);
	#pragma omp parallel 
	{
		printf("get_num_threads=%d\n", omp_get_num_threads());
	}
        MPI_Init_thread(&argc, &argv, required, &provided);
        comm = MPI_COMM_WORLD;
        if (provided != required) {
                fprintf(stderr, "no required provided\n");
                MPI_Abort(comm, -1);
        }
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	array = (char *) malloc(SIZE * sizeof(char));
	if (rank == 0) {
		printf("THREAD_NUM=%d\n", THREAD_NUM);
		for (k=0;k < MAX_STEPS; k++) {
			int m_size_per_thr = (SIZE * (k+1)) / (MAX_STEPS * THREAD_NUM);
			for (j=1; j < MAX_ITER; j++) {
				start[j] = MPI_Wtime();
				#pragma omp parallel for
				for (i=0; i< THREAD_NUM; i++) {
					MPI_Send(array+i*m_size_per_thr, /*SIZE/THREAD_NUM*/ m_size_per_thr, MPI_CHAR, 1, 0, comm);
				}

				MPI_Recv(empty, 0, MPI_CHAR, 1, 0, comm, MPI_STATUS_IGNORE);
				finish[j] = MPI_Wtime();
			}
			totalTime=0.0;
			for (j=1; j<MAX_ITER; j++) {
				totalTime+=finish[j]-start[j];
			}
			printf("%d\t%lf\n",(SIZE*(k+1))/(MAX_STEPS*THREAD_NUM), totalTime/(double)MAX_ITER);
			
				
		}
	}

	if (rank == 1) {
		for (k=0;k < MAX_STEPS; k++) {
			int m_size_per_thr = (SIZE * (k+1)) / (MAX_STEPS * THREAD_NUM);
			for (j=1; j < MAX_ITER; j++) {
				#pragma omp parallel for
				for (i=0; i< THREAD_NUM; i++) {
					MPI_Recv(array+i*m_size_per_thr, m_size_per_thr, MPI_CHAR, 0, 0, comm, MPI_STATUS_IGNORE);
				}
				MPI_Send(empty, 0, MPI_CHAR, 0, 0, comm);
			}
		}
	}

	if (rank == 0) {
	}
        MPI_Finalize();
        return 0;
}

