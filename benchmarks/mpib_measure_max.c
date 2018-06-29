#include <stdio.h>
#include <malloc.h>
#include "mpib_coll_benchmarks.h"
#include "mpib_output.h"

int MPIB_measure_max(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result) {
	int err = container->initialize(container, comm, root, M);
	int res;
	MPI_Allreduce(&err, &res, 1, MPI_INT, MPI_MAX, comm);
	if (res)
		return res;
	int rank;
	MPI_Comm_rank(comm, &rank);
	// maximum times are collected at root for statistical analysis
	double* T = NULL;
	if (rank == root)
		T = (double*)malloc(sizeof(double) * precision.max_reps);
	// everywhere
	int stop = 0;
	int reps = 0;
	// at root
	double sum = 0;
	double ci = 0;
	MPI_Barrier(comm);
	while (!stop && reps < precision.max_reps) {
		MPI_Barrier(comm);
		double time = MPI_Wtime();
		int err = container->execute(container, comm, root, M);
		time = MPI_Wtime() - time;
		int res;
		MPI_Reduce(&err, &res, 1, MPI_INT, MPI_MAX, root, comm);
		MPI_Reduce(&time, &T[reps], 1, MPI_DOUBLE, MPI_MAX, root, comm);
		if (rank == root)
			sum += T[reps];
		reps++;
		// statistical analysis is performed for at least 3 repetitions
		if (reps >= precision.min_reps && reps >= 3) {
			if (rank == root) {
				ci = MPIB_ci(precision.cl, reps, T);
				// stop if time is reliable or execution failed
				stop = ci * reps / sum < precision.eps && !res;
			}
			MPI_Bcast(&stop, 1, MPI_INT, root, comm);
		}
	}
	if (rank == root)
		free(T);
	err = container->finalize(container, comm, root);
	MPI_Allreduce(&err, &res, 1, MPI_INT, MPI_MAX, comm);
	if (res)
		return res;
	result->M = M;
	MPI_Bcast(&sum, 1, MPI_DOUBLE, root, comm);
	result->T = sum / reps;
	MPIB_max_wtick(comm, &result->wtick);
	result->reps = reps;
	MPI_Bcast(&ci, 1, MPI_DOUBLE, root, comm);
	result->ci = ci;
	return 0;
}

int MPIB_measure_max_adaptive(MPIB_coll_container* container, MPI_Comm comm, int root,
		MPIB_msgset msgset,	MPIB_precision precision, int* count, MPIB_result** results) {
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		if (MPIB_verbose)
			fprintf(stderr, "#adaptive max benchmark root = %d:", root);
		*count = 0;
		*results = NULL;
	}
	int stride = msgset.min_stride;
	int c = 0;
	int M = msgset.min_size;
	// use the same array and wtick value for communication experiments with different message sizes
	double* T = (double*)malloc(sizeof(double) * precision.max_reps);
	double wtick;
	MPIB_max_wtick(comm, &wtick);
	while (M < msgset.max_size && c < msgset.max_num) {
		if ((rank == root) && MPIB_verbose)
			fprintf(stderr, " %d", M);
		container->initialize(container, comm, root, M);
		int stop = 0;
		double sum = 0;
		int reps = 0;
		double ci = 0;
		while (!stop && reps < precision.max_reps) {
			MPI_Barrier(comm); MPI_Barrier(comm);
			double time = MPI_Wtime();
			int res = container->execute(container, comm, root, M);
			time = MPI_Wtime() - time;
			if (res != 0) return res;
			MPI_Allreduce(&time, &T[reps], 1, MPI_DOUBLE, MPI_MAX, comm);
			sum += T[reps];
			reps++;
			if (reps >= precision.min_reps && reps > 2)
				stop = (ci = MPIB_ci(precision.cl, reps, T)) * reps / sum < precision.eps;
		}
		container->finalize(container, comm, root);
		if (rank == root) {
			*count = c + 1;
			*results = (MPIB_result*)realloc(*results, sizeof(MPIB_result) * (c + 1));
			*results[c] = (MPIB_result){M, sum / reps, wtick, reps, ci};
		}
		if (MPIB_diff(*results[c], &*results[c - 2]) < msgset.max_diff)
			stride *= 2;
		else if (stride > msgset.min_stride)
			stride /= 2;
		c++;
	}
	free(T);
	if ((rank == root) && MPIB_verbose)
		fprintf(stderr, "\n");
	return 0;
}
