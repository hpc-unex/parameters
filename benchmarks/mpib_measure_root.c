#include <malloc.h>
#include "mpib_coll_benchmarks.h"
#include "mpib_utilities.h"

/*! The state of root timer */
typedef struct MPIB_root_timer
{
	/*! The communicator the barrier time was found for */
	MPI_Comm comm;
	/*! The barrier time at the current process */
	double barrier_time;
}
MPIB_root_timer;

/*! The state of root timer */
static MPIB_root_timer MPIB_root_timer_instance = {MPI_COMM_NULL, 0};

void MPIB_root_timer_init(MPI_Comm comm, int reps)
{
	MPIB_root_timer_instance.comm = comm;
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Barrier(comm); MPI_Barrier(comm);
	MPIB_root_timer_instance.barrier_time = MPI_Wtime();
	int i;
	for (i = 0; i < reps; i++)
		MPI_Barrier(comm);
	MPIB_root_timer_instance.barrier_time = (MPI_Wtime() - MPIB_root_timer_instance.barrier_time) / reps;
}

int MPIB_measure_root(MPIB_coll_container* container, MPI_Comm comm, int root, int M,
	MPIB_precision precision, MPIB_result* result)
{
	if (comm != MPIB_root_timer_instance.comm)
		MPIB_root_timer_init(comm, precision.max_reps);

	int rank;
	MPI_Comm_rank(comm, &rank);
	double* T = rank == root ? (double*)malloc(sizeof(double) * precision.max_reps) : NULL;

	container->initialize(container, comm, root, M);
	int stop = 0;
	double sum = 0;
	int reps = 0;
	double ci = 0;
	while (!stop && reps < precision.max_reps)
	{
		MPI_Barrier(comm); MPI_Barrier(comm);
		if (rank == root)
			T[reps] = MPI_Wtime();
		int res = container->execute(container, comm, root, M);
		if (res != 0) return res;
		MPI_Barrier(comm);
		if (rank == root)
			sum += T[reps] = MPI_Wtime() - T[reps] - MPIB_root_timer_instance.barrier_time;
		reps++;
		if (reps >= precision.min_reps && reps > 2)
		{
			if (rank == root)
				stop = (ci = MPIB_ci(precision.cl, reps, T)) * reps / sum < precision.eps;
			MPI_Bcast(&stop, 1, MPI_INT, root, comm);
		} 
	}
	container->finalize(container, comm, root);

	if (rank == root)
	{
		result->T = sum / reps;
		result->wtick = MPI_Wtick();
		result->reps = reps;
		result->ci = ci;
	}
	MPI_Bcast(&result, sizeof(MPIB_result), MPI_CHAR, root, comm);
	free(T);
	return 0;
}
