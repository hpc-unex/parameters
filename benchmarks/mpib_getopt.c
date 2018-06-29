#include "mpib_getopt.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "mpib_coll_containers.h"
#include "mpib_defs.h"
#include "mpib_output.h"
#include "mpib_shared.h"

void MPIB_getopt_help_default(int* exit) {
	*exit = 0;
}

void MPIB_getopt_multicore_default(int* multicore) {
	*multicore = 1;
}

const char* MPIB_getopt_help_options() {
	return "hv";
}

void MPIB_getopt_help_optarg(int c, const char* program, int* exit) {
	switch (c) {
		case 'h':
			fprintf(
				stderr,
"usage: mpirun [mpi options] %s [options]\n\
-h			help\n\
-v			verbose\n",
				program
			);
			*exit = 1;
			break;
		case 'v':
			MPIB_verbose = 1;
			break;
	}
}

void MPIB_getopt_help_bcast(int* exit, int root, MPI_Comm comm) {
	MPI_Bcast(exit, 1, MPI_INT, root, comm);
	MPI_Bcast(&MPIB_verbose, 1, MPI_INT, root, comm);
}

void MPIB_getopt_msgset_default(MPIB_msgset* msgset) {
	*msgset = (MPIB_msgset){0/*min_size*/, 204800/*max_size*/, 1024/*stride*/, 0.1/*max_diff*/, 64/*min_stride*/, 32000 /*threshold*/ ,0 /*commtype*/,100/*max_num*/};
}

const char* MPIB_getopt_msgset_options() {
	return "hm:M:S:d:s:T:C:n";
}

void MPIB_getopt_msgset_optarg(int c, MPIB_msgset* msgset) {
	switch (c) {
		case 'h':
			fprintf(
				stderr,
"-m I			minimum message size (default: %d)\n\
-M I			maximum message size (default: %d)\n\
-S I			stride between message sizes (default: %d)\n\
-d I			maximum relative difference: 0 < D < 1 (default: %le)\n\
-s I			minimum stride between message sizes (default: %d)\n\
-T I			threshold of message buffer (default: %d)\n\
-C I			Communicator (0 for tcp/ 1 for IB) (default: %d)\n\
-n I			maximum number of message sizes (default: %d)\n",
				msgset->min_size,
				msgset->max_size,
				msgset->stride,
				msgset->max_diff,
				msgset->min_stride,
				msgset->threshold,
				msgset->comm_type,
				msgset->max_num
			);
			break;
		case 'm':
			msgset->min_size = atoi(optarg);
			break;
		case 'M':
			msgset->max_size = atoi(optarg);
			break;
		case 'S':
			msgset->stride = atoi(optarg);
			break;
		case 'd':
			msgset->max_diff = atof(optarg);
			break;
		case 's':
			msgset->min_stride = atoi(optarg);
			break;
		case 'n':
			msgset->max_num = atoi(optarg);
			break;
		case 'T':
			msgset->threshold = atoi(optarg);
			break;
		case 'C':
			msgset->comm_type = atoi(optarg);
			break;
	}
}

void MPIB_getopt_msgset_bcast(MPIB_msgset* msgset, int root, MPI_Comm comm) {
	MPI_Bcast(msgset, sizeof(MPIB_msgset), MPI_CHAR, root, comm);
}

void MPIB_getopt_precision_default(MPIB_precision* precision) {
	*precision = (MPIB_precision){30/*min_reps*/, 1000/*max_reps*/, 0.95/*cl*/, 0.025/*eps*/};
}

const char* MPIB_getopt_precision_options() {
	return "hr:R:c:e:";
}

void MPIB_getopt_precision_optarg(int c, MPIB_precision* precision) {
	switch (c) {
		case 'h':
			fprintf(
				stderr,
"-r I			minimum number of repetitions (default: %d)\n\
-R I			maximum number of repetitions (default: %d)\n\
-c D			confidence level: 0 < D < 1 (default: %le)\n\
-e D			relative error: 0 < D < 1 (default: %le)\n",
				precision->min_reps,
				precision->max_reps,
				precision->cl,
				precision->eps
			);
			break;
		case 'r':
			precision->min_reps = atoi(optarg);
			if (precision->max_reps < precision->min_reps)
				precision->max_reps = precision->min_reps;
			break;
		case 'R':
			precision->max_reps = atoi(optarg);
			if (precision->min_reps > precision->max_reps)
				precision->min_reps = precision->max_reps;
			break;
		case 'c':
			precision->cl = atof(optarg);
			break;
		case 'e':
			precision->eps = atof(optarg);
			break;
	}
}

void MPIB_getopt_precision_bcast(MPIB_precision* precision, int root, MPI_Comm comm) {
	MPI_Bcast(precision, sizeof(MPIB_precision), MPI_CHAR, root, comm);
}

void MPIB_getopt_p2p_default(int* parallel, int* p2p_type, int* proc_num_spawned) {
	*parallel = 0;
	*p2p_type = 0;
	*proc_num_spawned = 2;
}

const char* MPIB_getopt_p2p_options() {
	return "p:P:x:";
}

void MPIB_getopt_p2p_optarg(int c, int* parallel, int* p2p_type, int* proc_num_spawned) {
	switch (c) {
		case 'h':
			fprintf(
				stderr,
"-p 0/1			parallel p2p benchmarking (default: %d)\n\
-P 0/1			standard P2P = 0 or scatter-gather P2P = 1 (default: %d); If using -P 1, run your tool from <install-dir>/bin \n\
-x D			number of processes to spawn at each node (default: %d)\n",
				*parallel, *p2p_type, *proc_num_spawned
			);
			break;
		case 'p':
			*parallel = atoi(optarg);
			break;
		case 'P':
			*p2p_type = atoi(optarg);
			break;
		case 'x':
			*proc_num_spawned = atoi(optarg);
			break;
	}
}

void MPIB_getopt_p2p_bcast(int* parallel, int *p2p_type, int* proc_num_spawned, int root, MPI_Comm comm) {
	MPI_Bcast(parallel, 1, MPI_INT, root, comm);
	MPI_Bcast(p2p_type, 1, MPI_INT, root, comm);
	MPI_Bcast(proc_num_spawned, 1, MPI_INT, root, comm);
}

void MPIB_getopt_coll_default(char* timing) {
	strcpy(timing, "max");
}

const char* MPIB_getopt_coll_options() {
	return "ht:";
}

void MPIB_getopt_coll_optarg(int c, char* timing) {
	switch (c) {
		case 'h':
			fprintf(
				stderr,
"-t S			timing: max, root, global (default: %s)\n",
				timing
			);
			break;
		case 't':
			strcpy(timing, optarg);
			break;
	}
}

void MPIB_getopt_coll_bcast(char* timing, MPIB_measure_coll* measure, int root, MPI_Comm comm)
{
	*measure = NULL;
	int rank;
	MPI_Comm_rank(comm, &rank);
	int len;
	if (rank == root)
		len = strlen(timing) + 1;
	MPI_Bcast(&len, 1, MPI_INT, root, comm);
	MPI_Bcast(timing, len, MPI_CHAR, root, comm);
	if (!strcmp(timing, "max"))
		*measure = MPIB_measure_max;
	if (!strcmp(timing, "root"))
		*measure = MPIB_measure_root;
	if (!strcmp(timing, "global"))
		*measure = MPIB_measure_global;
}
