/*!
 * \page sgv Test for the tree-based scatterv/gatherv
 * Test for tree-based scatterv/gatherv. Based on \latexonly\cite{Traff2004}\endlatexonly.
 */
#include <getopt.h>
#include <malloc.h>
#include <stdio.h>

#include "collectives/libmpib_coll.h"
#include "collectives/mpib_coll.h"

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	int exit = 0;
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		int c;
		while ((c = getopt(argc, argv, "h")) != -1) {
			switch (c) {
			case 'h':
				fprintf(
					stderr,
"usage: mpirun -np 8 sgv [options]\n\
-h			help\n"
				);
				exit = 1;
				break;
			}
		}
	}
	MPI_Bcast(&exit, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (exit) {
		MPI_Finalize();
		return 0;
	}
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	if (size != 8) {
		if (rank == 0)
			fprintf(stderr, "Number of processes must be 8\n");
		MPI_Finalize();
		return 0;
	}
	int counts[8] = {4, 2, 1, 1, 1, 2, 8 ,1};
	int displs[8];
	int count = 0;
	int i;
	for (i = 0; i < 8; i++) {
		displs[i] = i == 0 ? 0 : displs[i - 1] + counts[i - 1];
		count += counts[i];
	} 
	char* buffer = rank == 0 ? (char*)malloc(sizeof(char) * count) : (char*)malloc(sizeof(char) * counts[rank]);
	mpib_coll_verbose = 1;
	if (rank == 0)
		printf("#Binomial tree\n");
	MPIB_Scatterv_binomial(buffer, counts, displs, MPI_CHAR, buffer, counts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	if (rank == 0)
		printf("#Sorted binomial tree\n");
	MPIB_Scatterv_sorted_binomial_dsc(buffer, counts, displs, MPI_CHAR, buffer, counts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	if (rank == 0)
		printf("#Traff tree\n");
	MPIB_Scatterv_Traff(buffer, counts, displs, MPI_CHAR, buffer, counts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);
	free(buffer);
	MPI_Finalize();
	return 0;
}
