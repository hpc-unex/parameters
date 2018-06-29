/*!
 * \page sg-based-p2p Scatter-gather-based p2p benchmarks
 * Program for testing the correctness and benchmarking of
 * the modified scatter-gather-based p2p. The correctness is checked by verifying
 * the same data is received that is sent. The benchmarks are done by taking the maximum time
 * for each operation to complete from sender or receiver, and taking the minimum from a
 * number of iterations with this approach
 *
 * To run:
 * \code
 * cd <install-dir>/bin
 * mpirun -n 2 <src-dir>/MPIBlib/tests/sg-based-p2p
 * \endcode
 */
#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define MAX_ITER 16
#define MAX_STEPS 32

int main(int argc, char **argv) {
	int i;
	char * s_array = NULL;
	char * r_array = NULL;
	int debug_counter = 0;
	MPI_Comm comm;
	int rank;
	int j, k;
	double totalTime = 0.0;
	double minTime;

	MPI_Init(&argc, &argv);
	comm = MPI_COMM_WORLD;
	p2p_init(comm, 2);
	MPI_Comm_rank(comm, &rank);

	int SIZE = 1000000;
	//rank 0 initializes the data
	if (rank == 0) {
		s_array = (char *) malloc(SIZE * sizeof(char));
		for (i = 0; i < SIZE; i++) {
			s_array[i] = i%128 ;
		}
	}

	if (rank == 1)
		r_array = (char *) malloc(SIZE * sizeof(char));

	for (k = 0; k < MAX_STEPS; k++) {
		totalTime = 0.0;
		minTime = 10000.;
		for (j = 0; j < MAX_ITER; j++) {
			if ((rank == 1) && (j == 0)) {
				for (i = 0; i < SIZE; i++)
					r_array[i] = i%128 +1;
			}
			MPI_Barrier(comm);
			double start = MPI_Wtime();
			if (rank == 0) {
				MPI_Send(s_array, (SIZE * (k + 1)) / MAX_STEPS,
						MPI_CHAR, 1, 0, comm);
			} else if (rank == 1) {
				MPI_Recv(r_array, (SIZE * (k + 1)) / MAX_STEPS,
						MPI_CHAR, 0, 0, comm, MPI_STATUS_IGNORE);
			}
			double end = MPI_Wtime();
			if ((rank == 1) && (j == 0)) {
				debug_counter = 0;
				for (i = 0; i < SIZE; i++) {
					if (r_array[i] == ( i%128)) {
						debug_counter++;
					}
				}
				if (debug_counter != (SIZE * (k+1))/MAX_STEPS) {
					printf("Things are not looking good! Debug_counter = %d, rest is %d\n", debug_counter, SIZE * (k+1)/MAX_STEPS);
					for (i = 0; i < SIZE; i++) {
						putchar(r_array[i]);
					}
					putchar('\n');
				}
					
			}
			double duration = end - start;
			double max_duration;
			MPI_Reduce(&duration, &max_duration, 1, MPI_DOUBLE, MPI_MAX, 0,
					comm);
			if (rank == 0) {
				totalTime += max_duration;
				if (max_duration < minTime)
					minTime = max_duration;
			}
		}
		if (rank == 1)
			MPI_Send(&debug_counter, 1, MPI_INT, 0, 0, comm);
		if (rank == 0) {
			MPI_Recv(&debug_counter, 1, MPI_INT, 1, 0, comm, MPI_STATUS_IGNORE);
			printf("%d\t%lf\n", debug_counter, minTime);
		}
	}

	p2p_finalize();
	MPI_Finalize();
	return 0;
}

