#include <malloc.h>
#include "mpib_coll_benchmarks.h"
#include "mpib_utilities.h"
#include <gsl/gsl_math.h>

/*! The state of global timer */
typedef struct MPIB_global_timer {
	/*! The communicator the offset was found for */
	MPI_Comm comm;
	/*! The offset between clocks of the current and root processes */
	double* delta;
}
MPIB_global_timer;

/*! The state of global timer */
static MPIB_global_timer MPIB_global_timer_instance = {MPI_COMM_NULL, NULL};

void MPIB_global_timer_init(MPI_Comm comm, int parallel, int reps) {
	MPIB_global_timer_instance.comm = comm;
	free(MPIB_global_timer_instance.delta);
	int size;
	MPI_Comm_size(comm, &size);
	double* delta = MPIB_global_timer_instance.delta = (double*)calloc(size, sizeof(double));
	int* mpi_wtime_is_global;
	int flag;
#if MPI_VERSION < 2
	MPI_Attr_get(comm, MPI_WTIME_IS_GLOBAL, &mpi_wtime_is_global, &flag);
#else
	MPI_Comm_get_attr(comm, MPI_WTIME_IS_GLOBAL, &mpi_wtime_is_global, &flag);
#endif 
	if (!flag || !*mpi_wtime_is_global) {
		int rank;
		MPI_Comm_rank(comm, &rank);
		MPIB_pairs* pairs = MPIB_build_pairs(size);
		MPIB_pairs* iter_pairs = pairs;
		while (iter_pairs) {
			MPI_Barrier(comm);
			MPIB_pair* iter_pair = iter_pairs->list;
			while (iter_pair) {
				if (!parallel)
					MPI_Barrier(comm);
				int i = iter_pair->values[0];
				int j = iter_pair->values[1];
				if (rank == i || rank == j) {
					double delta_lb = -DBL_MAX;
					double delta_ub = DBL_MAX;
					double past = MPI_Wtime();
					int r;
					for (r = 0; r < reps; r++) {
						double time;
						double now;
						if (rank == i) {
							MPI_Send(&past, 1, MPI_DOUBLE, j, 0, comm);
							MPI_Recv(&time, 1, MPI_DOUBLE, j, 0, comm, MPI_STATUS_IGNORE);
							now = MPI_Wtime();
						}
						if (rank == j) {
							MPI_Recv(&time, 1, MPI_DOUBLE, i, 0, comm, MPI_STATUS_IGNORE);
							now = MPI_Wtime();
							MPI_Send(&now, 1, MPI_DOUBLE, i, 0, comm);
						}
						delta_lb = GSL_MAX_DBL(time - now, delta_lb);
						delta_ub = GSL_MIN_DBL(time - past, delta_ub);
						past = now;
					}
					if (rank == i)
						delta[j] = (delta_lb + delta_ub) / 2;
					if (rank == j)
						delta[i] = (delta_lb + delta_ub) / 2;
				}
				iter_pair = iter_pair->next;
			}
			iter_pairs = iter_pairs->next;
		}
		MPIB_free_pairs(pairs);
	}
}

int MPIB_measure_global(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result) {
	if (comm != MPIB_global_timer_instance.comm)
		MPIB_global_timer_init(comm, 1, precision.max_reps);
	int rank;
	MPI_Comm_rank(comm, &rank);
	double* T = rank == root ? (double*)calloc(precision.max_reps, sizeof(double)) : NULL;
	container->initialize(container, comm, root, M);
	int stop = 0;
	double sum = 0;
	int reps = 0;
	double ci = 0;
	while (!stop && reps < precision.max_reps) {
		MPI_Barrier(comm); MPI_Barrier(comm);
		double start = MPI_Wtime();
		int res = container->execute(container, comm, root, M);
		if (res != 0) return res;
		double tmp = MPI_Wtime() - MPIB_global_timer_instance.delta[root];
		double finish;
		MPI_Reduce(&tmp, &finish, 1, MPI_DOUBLE, MPI_MAX, root, comm);
		if (rank == root)
			sum += T[reps] = finish - start;
		reps++;
		if (reps >= precision.min_reps && reps > 2) {
			if (rank == root)
				stop = (ci = MPIB_ci(precision.cl, reps, T)) * reps / sum < precision.eps;
			MPI_Bcast(&stop, 1, MPI_INT, root, comm);
		} 
	}
	container->finalize(container, comm, root);
	MPIB_max_wtick(comm, &result->wtick);
	if (rank == root) {
		result->T = sum / reps;
		result->reps = reps;
		result->ci = ci;
	}
	MPI_Bcast(result, sizeof(MPIB_result), MPI_CHAR, root, comm);
	free(T);
	return 0;
}
