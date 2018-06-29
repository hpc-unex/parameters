#include <malloc.h>
#include "mpib_benchmarks.h"
#include "mpib_p2p_containers.h"
#include "mpib_utilities.h"
#include <gsl/gsl_statistics_double.h>

/*! The state of bcast timer */
typedef struct MPIB_bcast_timer
{
	/*! The communicator the offset was found for */
	MPI_Comm comm;
	/*! The offset between clocks of the current and root processes */
	MPIB_result* results;
}
MPIB_bcast_timer;

/*! The state of bcast timer */
static MPIB_bcast_timer MPIB_bcast_timer_instance = {MPI_COMM_NULL, NULL};

void MPIB_bcast_timer_init(MPI_Comm comm, int parallel, MPIB_precision precision)
{
	MPIB_bcast_timer_instance.comm = comm;
	free(MPIB_bcast_timer_instance.results);
	int size;
	MPI_Comm_size(comm, &size);
	MPIB_bcast_timer_instance.results = (MPIB_result*)malloc(sizeof(MPIB_result) * MPIB_C2(size));
	MPIB_p2p_container* container = MPIB_roundtrip_container_alloc(MPI_Send, MPI_Recv);
	MPIB_measure_allp2p(container, comm, 0, parallel, precision, MPIB_bcast_timer_instance.results); 
	MPIB_roundtrip_container_free(container);
}

void MPIB_measure_bcast(MPIB_Bcast bcast, MPI_Comm comm, int root, int M,
	int max_reps, MPIB_result* result)
{
	if (comm != MPIB_bcast_timer_instance.comm)
		MPIB_bcast_timer_init(comm, 1, (MPIB_precision){max_reps, max_reps, 0});

	int rank;
	MPI_Comm_rank(comm, &rank);
	int size;
	MPI_Comm_size(comm, &size);
	double* T = rank == root ? (double*)calloc(size, sizeof(double)) : NULL;
	char* buffer = (char*)malloc(sizeof(char) * M);
	MPI_Barrier(comm);
	int i;
	for (i = 0; i < size; i++)
	{
		if (i != root)
		{
			if (rank == root)
				T[i] = MPI_Wtime();
			int reps;
			for (reps = 0; reps < max_reps; reps++)
			{
				if (rank == root)
				{
					bcast(buffer, M, MPI_CHAR, root, comm);
					MPI_Recv(NULL, 0, MPI_CHAR, i, 0, comm, MPI_STATUS_IGNORE);
				}
				else if (rank == i)
				{
					bcast(buffer, M, MPI_CHAR, root, comm);
					MPI_Send(NULL, 0, MPI_CHAR, i, 0, comm);
				}
				else
					bcast(buffer, M, MPI_CHAR, root, comm);
			}
			if (rank == root)
				T[i] = (MPI_Wtime() - T[i]) / max_reps -
					MPIB_bcast_timer_instance.results[MPIB_IJ2INDEX(size, root, i)].T / 2;
		}
	}
	free(buffer);

	if (rank == root)
		result->T = gsl_stats_max(T, 1, size);
	MPI_Bcast(&result->T, 1, MPI_DOUBLE, root, comm);
	MPIB_max_wtick(comm, &result->wtick);
	result->reps = max_reps;
	result->ci = 0;
	free(T);
}
