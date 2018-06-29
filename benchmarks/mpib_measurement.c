#include "mpib_measurement.h"
#include <malloc.h>
#include <string.h>
#include <gsl/gsl_heapsort.h>
#include <math.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_statistics_double.h>

void MPIB_Comm(MPI_Comm comm, MPI_Comm* newcomm) {
	// collect processor names
	char name[MPI_MAX_PROCESSOR_NAME];
	int len;
	MPI_Get_processor_name(name, &len);
	int size;
	MPI_Comm_size(comm, &size);
	char* names = (char*)malloc(sizeof(char) * MPI_MAX_PROCESSOR_NAME * size);
	MPI_Allgather(name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, names, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, comm);
	// indirectly sort processor names
	size_t* indices = (size_t*)malloc(sizeof(size_t) * size);
	gsl_heapsort_index(indices, names, size, sizeof(char) * MPI_MAX_PROCESSOR_NAME, (int(*)(const void*, const void*))strcmp);
	// colour: (0, processor index) for the first process on each processor, MPI_UNDEFINED elsewhere
	int color = MPI_UNDEFINED;
	int key = 0;
	int rank;
	MPI_Comm_rank(comm, &rank);
	int i = 0;
	// colour until the current process is found
	do
		if (i == 0 || strcmp(&names[MPI_MAX_PROCESSOR_NAME * indices[i - 1]], &names[MPI_MAX_PROCESSOR_NAME * indices[i]])) {
			if (indices[i] == rank) {
				// the first process on the processor
				color = 0;
			} else {
				// the next processor
				key++;
			}
		}
	while (indices[i++] != rank);
	free(names);
	free(indices);
	// split the communicator
	MPI_Comm_split(comm, color, key, newcomm);
}

double MPIB_diff(MPIB_result result, MPIB_result results[2]) {
	return fabs(1 - result.T * (results[1].M - results[0].M) /
		(results[1].T * (result.M - results[0].M) - results[0].T * (result.M - results[1].M)));
}

void MPIB_max_wtick(MPI_Comm comm, double* wtick) {
	double tmp = MPI_Wtick();
	MPI_Allreduce(&tmp, wtick, 1, MPI_DOUBLE, MPI_MAX, comm); 
}

double MPIB_ci(double cl, int reps, double* T) {
	return fabs(gsl_cdf_tdist_Pinv(cl, reps - 1)) * gsl_stats_sd(T, 1, reps) / sqrt(reps); 
}
