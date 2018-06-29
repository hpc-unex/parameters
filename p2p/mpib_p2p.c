#include "mpib_p2p.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 0 means standard p2p, 1 means modified p2p
int MPIB_p2p_type = 0;

// Default number of processes to spawn at each node for modified p2p
int PROC_NUM_SPAWNED = 2;

// A global variable used to get the number of processes in total
// used for a single modified point-to-point communication between 2 nodes
// This number is the same each time once we have spawned the processes everywhere
int TOTAL_PROC_NUM = 0;

// Used only to create intracomm
MPI_Comm intercomm = MPI_COMM_NULL;

// Global communicator containing both the initial processes
// as well as all spawned processes
MPI_Comm intracomm = MPI_COMM_NULL;

// Used for control messages between any of the initial processes and any of the spawned processes
// This includes information about the type of communication or a termination signal
MPI_Comm control_intracomm = MPI_COMM_NULL;

// This array  holds the names of the machine hosting each rank:
// i.e. all_names[i] = hostname of machine where process i runs
char *all_names = NULL;

// Used for non-blocking point-to-point and waitall routines.
MPI_Request* global_reqs = NULL;

// Counter for global_reqs
int global_reqs_counter = 0;

int p2p_init(MPI_Comm comm, int proc_num_spawned) {
	//set global variable to always use modified p2p
	MPIB_p2p_type = 1;
	PROC_NUM_SPAWNED = proc_num_spawned;
	TOTAL_PROC_NUM = 2 + 2 * PROC_NUM_SPAWNED;
	char spawned_prog[64];
	strcpy(spawned_prog, "./p2p_forward");
	MPI_Comm_get_parent(&intercomm);
	int merge_flag;
	if (intercomm == MPI_COMM_NULL) {
		merge_flag = 0;
		/*
		 MPI_Info info;
		 MPI_Info_create(&info);
		 MPI_Info_set(info, "host", name);
		 */

		int size;
		MPI_Comm_size(comm, &size);
		int err_codes[size * PROC_NUM_SPAWNED];
		MPI_Comm_spawn(spawned_prog, MPI_ARGV_NULL, size * PROC_NUM_SPAWNED,
				MPI_INFO_NULL/*info*/, 0, comm, &intercomm, err_codes);
	} else {
		merge_flag = 1;
	}

	MPI_Intercomm_merge(intercomm, merge_flag, &intracomm);
	MPI_Comm_dup(intracomm, &control_intracomm);
	MPI_Bcast(&PROC_NUM_SPAWNED, 1, MPI_INT, 0/*root*/, intracomm);

	int real_length;
	char name[128];
	MPI_Get_processor_name(name, &real_length);
	int size, rank;
	MPI_Comm_size(intracomm, &size);
	MPI_Comm_rank(intracomm, &rank);
	all_names = (char *) malloc(sizeof(char) * 128 * size);
	//every process gets every other process's hostname
	MPI_Allgather(name, 128, MPI_CHAR, all_names, 128, MPI_CHAR, intracomm);
	return 0;
}

int signal_spawned_procs(int* rank_list, int rank_list_size, int flag, int source, int target, int sendcount, int rest);

int p2p_finalize() {
	int size, initial_size, rank;
	MPI_Comm_rank(intracomm, &rank);
	if (rank == 0) {
		MPI_Comm comm = MPI_COMM_WORLD;
		MPI_Comm_size(intracomm, &size);
		MPI_Comm_size(comm, &initial_size);
		int rank_list_size = size-initial_size;
		int rank_list[rank_list_size];
		int i;
		for (i = 0; i < rank_list_size; i++)
			rank_list[i] = initial_size+i;
		signal_spawned_procs(rank_list, rank_list_size, 2/*terminate flag*/, 0, 0, 0, 0);
	}
	MPI_Comm_free(&intracomm);
	MPI_Comm_free(&control_intracomm);
	MPI_Comm_free(&intercomm);
	return 0;
}

int MPIB_Send_sg(void *buf, int count, MPI_Datatype datatype, int dest,
		int tag, MPI_Comm comm, int MPIB_p2p_blocking);

int MPIB_Send(void *buf, int count, MPI_Datatype datatype, int dest,
		int tag, MPI_Comm comm) {
	if (MPIB_p2p_type == 0)
		return MPI_Send(buf, count, datatype, dest, tag, comm);
	else if (MPIB_p2p_type == 1) {
		return MPIB_Send_sg(buf, count, datatype, dest, tag, comm, 1);
	}
	fprintf(stderr, "Wrong MPIB_p2p_type = %d\n", MPIB_p2p_type);
	return -1;

}

int MPIB_Isend(void *buf, int count, MPI_Datatype datatype, int dest,
		int tag, MPI_Comm comm, MPI_Request *request) {
	if (MPIB_p2p_type == 0)
		return MPI_Isend(buf, count, datatype, dest, tag, comm, request);
	else if (MPIB_p2p_type == 1) {
		*request = MPI_REQUEST_NULL;
		return MPIB_Send_sg(buf, count, datatype, dest, tag, comm, 0);
	}
	fprintf(stderr, "Wrong MPIB_p2p_type = %d\n", MPIB_p2p_type);
	return -1;
}

int MPIB_Recv_sg(void *buf, int count, MPI_Datatype datatype, int source,
		int tag, MPI_Comm comm, MPI_Status *status, int MPIB_p2p_blocking);

int MPIB_Recv(void *buf, int count, MPI_Datatype datatype, int source,
		int tag, MPI_Comm comm, MPI_Status *status) {
	if (MPIB_p2p_type == 0)
		return MPI_Recv(buf, count, datatype, source, tag, comm, status);
	else if (MPIB_p2p_type == 1)
		return MPIB_Recv_sg(buf, count, datatype, source, tag, comm, status, 1);
	fprintf(stderr, "Wrong MPIB_p2p_type = %d\n", MPIB_p2p_type);
	return -1;
}

int MPIB_Irecv(void *buf, int count, MPI_Datatype datatype, int source,
		int tag, MPI_Comm comm, MPI_Request *request) {
	if (MPIB_p2p_type == 0)
		return MPI_Irecv(buf, count, datatype, source, tag, comm, request);
	else if (MPIB_p2p_type == 1) {
		*request = MPI_REQUEST_NULL;
		return MPIB_Recv_sg(buf, count, datatype, source, tag, comm,
				MPI_STATUS_IGNORE, 0);
	}
	fprintf(stderr, "Wrong MPIB_p2p_type = %d\n", MPIB_p2p_type);
	return -1;
}

int MPIB_Waitall(int count, MPI_Request *array_of_requests,
		MPI_Status *array_of_statuses) {
	if (MPIB_p2p_type == 0) {
		return MPI_Waitall(count, array_of_requests, array_of_statuses);
	}
	//if we are doing our own p2p communication, then we ignore all the input arguments.
	//Instead, we use our internal request list
	else if (MPIB_p2p_type == 1) {
		int retValue = MPI_Waitall(global_reqs_counter, global_reqs, MPI_STATUSES_IGNORE);
		free(global_reqs);
		global_reqs = NULL;
		global_reqs_counter = 0;
		return retValue;
	}
	fprintf(stderr, "Wrong MPIB_p2p_type = %d\n", MPIB_p2p_type);
	return -1;
}
