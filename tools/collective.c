/*!
 * \page collective Collective benchmark
 * Performs regular universal or operation-specific collective benchmark.
 * Basic arguments are described in \ref getopt.
 *
 * \b Example:
 * \code
 * $ mpirun -n 16 collective -O MPIB_Scatter_binomial -l libmpib_coll.so -m 1024 -M 2049 > collective.out
 * \endcode
 * will benchmark the default binomial tree scatter operation with message sizes per process of 1024 and 2048
 *
 * \b collective.plot draws the graph of the execution time (sec) against message size (kb) with error bars.
 * - input: collective.out
 * - output: collective.eps
 * 
 * Using the gnuplot script:
 * \code
 * $ gnuplot collective.plot
 * \endcode
 */
#include "config.h"

#include <dlfcn.h>
#include <getopt.h>
#include "p2p/mpib_p2p.h"
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "benchmarks/mpib.h"
#include "p2p/mpib_p2p.h"

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	int multicore = 0;
	MPIB_getopt_multicore_default(&multicore);
#ifndef MPIB_DEBUG
	if (!multicore) {
		MPIB_Comm(MPI_COMM_WORLD, &comm);
		if (comm == MPI_COMM_NULL) {
			MPI_Finalize();
			return 0;
		}
	}
#endif

	int exit;
	MPIB_getopt_help_default(&exit);
	MPIB_msgset msgset;
	MPIB_getopt_msgset_default(&msgset);
	MPIB_precision precision;
	MPIB_getopt_precision_default(&precision);
	char timing[256];
	int p2p_type, parallel, proc_num_spawned = 0;
	MPIB_getopt_p2p_default(&parallel, &p2p_type, &proc_num_spawned);
	MPIB_getopt_coll_default(timing);
	char library[256] = "";
	char subopts[256] = "";
	void* handle = NULL;
	char operation[256] = "";
	int size;
	MPI_Comm_size(comm, &size);
	double* factors = (double*)malloc(sizeof(double) * size);
	int i;
	for (i = 0; i < size; i++)
		factors[i] = 1;

	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == 0) {
		int c;
		char options[256] = "l:o:O:f:";
		strcat(options, MPIB_getopt_help_options());
		strcat(options, MPIB_getopt_msgset_options());
		strcat(options, MPIB_getopt_precision_options());
		strcat(options, MPIB_getopt_coll_options());
		strcat(options, MPIB_getopt_p2p_options());
		while ((c = getopt(argc, argv, options)) != -1) {
			MPIB_getopt_help_optarg(c, "collective", &exit);
			MPIB_getopt_msgset_optarg(c, &msgset);
			MPIB_getopt_precision_optarg(c, &precision);
			MPIB_getopt_coll_optarg(c, timing);
			MPIB_getopt_p2p_optarg(c, &parallel, &p2p_type, &proc_num_spawned);
			switch (c) {
				case 'h':
					fprintf(
						stderr,
"-l S			shared library (required: a full path or relative to LD_LIBRARY_PATH)\n\
-o S			suboptions (comma separated options for the shared library)\n\
-O S			collective operation defined in the shared library (required: the name must contain Bcast, Scatter, etc)\n\
-f S			input file containing the multiplying factors for scatterv/gatherv (default: all factors = 1)\n\
-M			this flag is required for multi-core experiments\n");
					break;
				case 'l':
					strcpy(library, optarg);
					break;
				case 'o':
					strcpy(subopts, optarg);
					break;
				case 'O':
					strcpy(operation, optarg);
					break;
				case 'f': {
					FILE* stream = fopen(optarg, "r");
					if (stream == NULL) {
						fprintf(stderr, "Cannot open file %s for reading\n", optarg);
						exit = 1;
					} else {
						int i;
						for (i = 0; i < size; i++)
							fscanf(stream, "%le\n", &factors[i]);
						fclose(stream);
					}
					break;
				}
			}
		}
	}

	MPIB_getopt_help_bcast(&exit, 0, comm);
	if (exit) {
		MPI_Finalize();
		return 0;
	}
	MPIB_print_processors(comm);
	MPIB_getopt_msgset_bcast(&msgset, 0, comm);
	MPIB_getopt_precision_bcast(&precision, 0, comm);

	MPIB_getopt_p2p_bcast(&parallel, &p2p_type, &proc_num_spawned, 0, comm);
	if (p2p_type == 1)
		p2p_init(comm, proc_num_spawned);

	MPIB_measure_coll measure;
	MPIB_getopt_coll_bcast(timing, &measure, 0, comm);
	if (!measure) {
		if (rank == 0)
			fprintf(stderr, "Undefined measurement method: %s\n", timing);
		MPI_Abort(comm, -1);
	}
	MPI_Bcast(factors, size, MPI_DOUBLE, 0, comm);

	int len;
	if (rank == 0)
		len = strlen(library) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, comm);
	MPI_Bcast(library, len, MPI_CHAR, 0, comm);
	handle = dlopen(library, RTLD_LAZY);
	if (handle == NULL) {
		fprintf(stderr, "%d: %s\n", rank, dlerror());
		MPI_Abort(comm, -1);
	}
	if (rank == 0)
		len = strlen(subopts) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, comm);
	MPI_Bcast(subopts, len, MPI_CHAR, 0, comm);
	MPIB_collectives_initialize initialize = dlsym(handle, MPIB_COLLECTIVES_INITIALIZE);
	if (initialize != NULL) {
		int err = initialize(comm, subopts);
		if (err){
			fprintf(stderr, "%d: initialization of %s failed\n", rank, library);
			MPI_Abort(comm, err);
		}
	}
			
	if (rank == 0)
		len = strlen(operation) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, comm);
	MPI_Bcast(operation, len, MPI_CHAR, 0, comm);
	void (*func) = dlsym(handle, operation);
	if (func == NULL) {
		fprintf(stderr, "%d: Operation %s not found in library %s\n", rank, operation, library);
		MPI_Abort(comm, -1);
	}
	MPIB_coll_container* container = NULL;
	if (strstr(operation, "Bcast"))
		container = (MPIB_coll_container*)MPIB_Bcast_container_alloc((MPIB_Bcast)func);
	if (strstr(operation, "Reduce"))
		container = (MPIB_coll_container*)MPIB_Reduce_container_alloc((MPIB_Reduce)func);
	if (strstr(operation, "catter") && !strstr(operation, "catterv"))
		container = (MPIB_coll_container*)MPIB_Scatter_container_alloc((MPIB_Scatter)func);
	if (strstr(operation, "ather") && !strstr(operation, "atherv"))
		container = (MPIB_coll_container*)MPIB_Gather_container_alloc((MPIB_Gather)func);
	if (strstr(operation, "catterv"))
		container = (MPIB_coll_container*)MPIB_Scatterv_container_alloc((MPIB_Scatterv)func, factors);
	if (strstr(operation, "atherv"))
		container = (MPIB_coll_container*)MPIB_Gatherv_container_alloc((MPIB_Gatherv)func, factors);
	if (strstr(operation, "Comm_dup_free"))
		container = (MPIB_coll_container*)MPIB_Comm_dup_free_container_alloc();
	if (!container) {
		fprintf(stderr, "%d: Undefined collective operation: %s\n", rank, operation);
		MPI_Abort(comm, -1);
	}

	if (rank == 0)
		MPIB_print_coll_th(operation, timing, size, 0, precision);
	double time = 0;
	int M;
	for (M = msgset.min_size; M < msgset.max_size; M += msgset.stride) {
		MPIB_result result;
		int start = MPI_Wtime();
		int err = measure(container, comm, 0, M, precision, &result);
		if (err) {
			fprintf(stderr, "%d: measure routine failed in collective\n", rank);
			MPI_Abort(comm, err);
		}
		time += MPI_Wtime() - start;
		if (rank == 0)
			MPIB_print_coll_tr(M, result);
	}
	if (rank == 0)
		printf("#total benchmarking time: %le\n", time);

	MPIB_coll_container_free(container);

	MPIB_collectives_finalize finalize = dlsym(handle, MPIB_COLLECTIVES_FINALIZE);
	if (finalize != NULL) {
		int err = finalize(comm);
		if (err){
			fprintf(stderr, "%d: finalization of %s failed\n", rank, library);
			MPI_Abort(comm, err);
		}
	}
	dlclose(handle);

	free(factors);

#ifndef MPIB_DEBUG
	if (!multicore) {
		MPI_Comm_free(&comm);
	}
#endif

	if (p2p_type == 1)
		p2p_finalize();
	MPI_Finalize();
	return 0;
}
