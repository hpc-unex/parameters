#include "mpib_output.h"
#include <stdio.h>

int MPIB_verbose = 0;

void MPIB_print_processors(MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	char name[MPI_MAX_PROCESSOR_NAME];
	int len;
	MPI_Get_processor_name(name, &len);
	if (rank == 0)
	{
		printf("#Processors\n#rank\tprocessor\n");
		printf("#%d\t%s\n", rank, name);
		int size;
		MPI_Comm_size(comm, &size);
		int i;
		for (i = 1; i < size; i++)
		{
			MPI_Recv(&rank, 1, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
			MPI_Recv(name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, i, 0, comm, MPI_STATUS_IGNORE);
			printf("#%d\t%s\n", rank, name);
		}
		printf("#\n");
	}
	else
	{
		MPI_Send(&rank, 1, MPI_INT, 0, 0, comm);
		MPI_Send(name, MPI_MAX_PROCESSOR_NAME, MPI_CHAR, 0, 0, comm);
	}
}

void MPIB_print_precision(MPIB_precision precision)
{
	printf("#Precision\n");
	printf("#min_reps\t%d\n", precision.min_reps); 
	printf("#max_reps\t%d\n", precision.max_reps); 
	printf("#cl\t\t%le\n", precision.cl); 
	printf("#eps\t\t%le\n", precision.eps); 
	printf("#\n");
}

void MPIB_print_msgset(MPIB_msgset msgset)
{
	printf("#Message set\n");
	printf("#min_size\t%d\n", msgset.min_size); 
	printf("#max_size\t%d\n", msgset.max_size); 
	printf("#stride\t\t%d\n", msgset.stride);
	printf("#max_diff\t%le\n", msgset.max_diff);
	printf("#min_stride\t%d\n", msgset.min_stride);
	printf("#max_num\t%d\n", msgset.max_num);
	printf("#\n");
}

void MPIB_print_result_th() {
	printf("#msg\ttime\t\twtick\treps\tci\n");
}

void MPIB_print_result_tr(MPIB_result result) {
	printf("%d\t%le\t%d\t%d\t%le\n",
		result.M,
		result.T,
		result.wtick < result.T,
		result.reps,
		result.ci
	);
	fflush(stdout);
}

void MPIB_print_coll(const char* operation, const char* timing)
{
	printf("#operation\t%s\n", operation);
	printf("#timing\t\t%s\n", timing);
	printf("#\n");
}

void MPIB_print_p2p(int parallel)
{
	printf("#%s\n", parallel ? "parallel" : "sequential");
	printf("#\n");
}

void MPIB_print_p2p_table(int M, int parallel, MPIB_precision precision, int n, MPIB_result* results)
{
	printf("#msg\t\t%d\n", M);
	MPIB_print_p2p(parallel);
	MPIB_print_precision(precision);
	printf
	(
"#p2p\t\
time\t\t\
wtick\t\
reps\t\
ci\n"
	);
	int i, index;
	for(i = 0, index = 0; i < n - 1; i++)
	{
		int j;
		for(j = i + 1; j < n; j++, index++)
			printf
			(
"%d-%d\t\
%le\t\
%d\t\
%d\t\
%le\n",
				i, j,
				results[index].T,
				results[index].wtick < results[index].T,
				results[index].reps,
				results[index].ci
			);
	}
	fflush(stdout);
}

void MPIB_print_p2p_th(int parallel, MPIB_precision precision, int n)
{
	printf("#%s\n", parallel ? "parallel" : "sequential");
	MPIB_print_precision(precision);
	printf("#\t");
	int i, index;
	for(i = 0, index = 0; i < n - 1; i++)
	{
		int j;
		for(j = i + 1; j < n; j++, index++)
			printf("%d-%d\t\t\t\t\t\t", i, j);
	}
	printf("\n#msg\t");
	for(i = 0, index = 0; i < n - 1; i++)
	{
		int j;
		for(j = i + 1; j < n; j++, index++)
			printf
			(
"time\t\t\
wtick\t\
reps\t\
ci\t\t"
			);
	}
	printf("\n");
}

void MPIB_print_p2p_tr(int M, int n, MPIB_result* results)
{
	printf("%d\t", M);
	int i, index;
	for(i = 0, index = 0; i < n - 1; i++)
	{
		int j;
		for(j = i + 1; j < n; j++, index++)
			printf
			(
"%le\t\
%d\t\
%d\t\
%le\t",
				results[index].T,
				results[index].wtick < results[index].T,
				results[index].reps,
				results[index].ci
			);
	}
	printf("\n");
	fflush(stdout);
}

void MPIB_print_coll_th(const char* operation, const char* timing, int n, int root, MPIB_precision precision)
{
	printf("#nodes\t\t%d\n", n);
	printf("#root\t\t%d\n", root);
	MPIB_print_coll(operation, timing);
	MPIB_print_precision(precision);
	printf
	(
"#msg\t\
time\t\t\
wtick\t\
reps\t\
ci\n"
	);
}

void MPIB_print_coll_tr(int M, MPIB_result result)
{
	printf
	(
"%d\t\
%le\t\
%d\t\
%d\t\
%le\n",
		M,
		result.T,
		result.wtick < result.T,
		result.reps,
		result.ci
	);
	fflush(stdout);
}

