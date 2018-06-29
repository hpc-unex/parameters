/*!
 * \page collective_test Test for MPI collective operations
 * Performs tests for the implementations of MPI collective operations with different root
 */
#include <dlfcn.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "mpib_test.h"

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);

	int exit;
	MPIB_getopt_help_default(&exit);
	char library[256] = "";
	char subopts[256] = "";
	void* handle = NULL;
	char operation[256] = "";

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		int c;
		char options[256] = "l:o:O:";
		strcat(options, MPIB_getopt_help_options());
		while ((c = getopt(argc, argv, options)) != -1) {
			MPIB_getopt_help_optarg(c, "collective_test", &exit);
			switch (c) {
				case 'h':
					fprintf(
						stderr,
"-l S			shared library (required: a full path or relative to LD_LIBRARY_PATH)\n\
-o S			suboptions (MPI_COMM_WORLDa separated options for the shared library)\n\
-O S			collective operation defined in the shared library (required: the name must contain Bcast, Scatter, etc)\n"
					);
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
			}
		}
	}
	MPIB_getopt_help_bcast(&exit, 0, MPI_COMM_WORLD);
	if (exit) {
		MPI_Finalize();
		return 0;
	}

	int len;
	if (rank == 0)
		len = strlen(library) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(library, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	handle = dlopen(library, RTLD_LAZY);
	if (handle == NULL) {
		if (rank == 0)
			fprintf(stderr, "%s\n", dlerror());
		MPI_Finalize();
		return 0;
	}
	if (rank == 0)
		len = strlen(subopts) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(subopts, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	void (*func) = dlsym(handle, MPIB_COLLECTIVES_INITIALIZE);
	if (func != NULL)
		((MPIB_collectives_initialize)func)(MPI_COMM_WORLD, subopts);
	if (rank == 0)
		len = strlen(operation) + 1;
	MPI_Bcast(&len, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(operation, len, MPI_CHAR, 0, MPI_COMM_WORLD);
	func = dlsym(handle, operation);
	if (func == NULL) {
		if (rank == 0)
			fprintf(stderr, "Operation %s not found in library %s\n", operation, library);
		MPI_Finalize();
		return 0;
	}

	if (rank == 0)
	{
		printf("#Test of %s\n", operation);
		printf("#root\tresult\n");
	}
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int root;
	for (root = 0; root < size; root++)
	{
		int res;
		if (strstr(operation, "Bcast"))
			MPIB_test_bcast((MPIB_Bcast)func, MPI_COMM_WORLD, root, &res);
		if (strstr(operation, "Reduce"))
			MPIB_test_reduce((MPIB_Reduce)func, MPI_COMM_WORLD, root, &res);
		if (strstr(operation, "Scatter") && !strstr(operation, "Scatterv"))
			MPIB_test_scatter((MPIB_Scatter)func, MPI_COMM_WORLD, root, &res);
		if (strstr(operation, "Gather") && !strstr(operation, "Gatherv"))
			MPIB_test_gather((MPIB_Gather)func, MPI_COMM_WORLD, root, &res);
		if (strstr(operation, "Scatterv"))
			MPIB_test_scatterv((MPIB_Scatterv)func, MPI_COMM_WORLD, root, &res);
		if (strstr(operation, "Gatherv"))
			MPIB_test_gatherv((MPIB_Gatherv)func, MPI_COMM_WORLD, root, &res);
		if (rank == 0)
			printf("#%d\t%d\n", root, res);
	}
	if (rank == 0)
		printf("#\n");

	func = dlsym(handle, MPIB_COLLECTIVES_FINALIZE);
	if (func != NULL)
		((MPIB_collectives_finalize)func)(MPI_COMM_WORLD);
	dlclose(handle);

	MPI_Finalize();
	return 0;
}
