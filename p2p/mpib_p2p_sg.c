#include "mpib_p2p.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


extern char *all_names;
extern int PROC_NUM_SPAWNED;
extern int TOTAL_PROC_NUM;
extern MPI_Comm intercomm;
extern MPI_Comm intracomm;
extern MPI_Comm control_intracomm;
extern MPI_Request* global_reqs;
extern int global_reqs_counter;

/*!
 * This routine populates the list of all processes run on the
 * hosts participating in the point-to-point communication between
 * sender and receiver
 * \param rank_list with size = all processes involved in 2 node communication
 * \param sender rank of sender
 * \param receiver rank of receiver
 */
void populate_rank_list(int* rank_list, int sender,
		int receiver, int comm_size) {

	int real_length;
	char name[128];
	int i;
	MPI_Get_processor_name(name, &real_length);
	char sender_name[128];
	char receiver_name[128];
	strncpy(sender_name, &all_names[sender * 128], 128);
	strncpy(receiver_name, &all_names[receiver * 128], 128);
	int j = 0;
	for (i = 0; i < comm_size; i++) {
		if (!strcmp(sender_name, &all_names[i * 128]) || !strcmp(receiver_name,
				&all_names[i * 128])) {
			rank_list[j++] = i;
		}
	}
	if (j != TOTAL_PROC_NUM) {
		fprintf(stderr, "Error in populate_rank_list, j =  %d, comm_size = %d\n", j, comm_size);
		exit(-1);
	}
}

/*!
 * Send out a message to all spawned MPI processes to let them know
 * how much data to forward:
 * if flag = 1: then this is a modified point-to-point communication and
 * 	the rank_list contains all processes on the two communicating nodes
 * if flag = 2: then this is a terminating call to all slave processes on all nodes
 * The call is blocking since it is usually followed by non-blocking communication which depends on this synchronisation
 */
int signal_spawned_procs(int* rank_list, int rank_list_size, int flag, int source, int target, int sendcount, int rest) {
	MPI_Request reqs[rank_list_size];
	int info[4];
	info[0] = flag;
	info[1] = target;
	info[2] = sendcount;
	info[3] = rest;
	int i;
	for (i = 0; i < rank_list_size; i++) {
		if ((rank_list[i] != source) && (rank_list[i] != target))
			MPI_Isend(&info, 4, MPI_INT, rank_list[i], 0, control_intracomm,
					&reqs[i]);
		else
			reqs[i] = MPI_REQUEST_NULL;
	}
	MPI_Waitall(rank_list_size, reqs, MPI_STATUSES_IGNORE);
	return 0;
}

/*!
 * This method is the scatter-gather implementation of point-to-point communication. It should be called by both sender and receiver -
 * other than actual MPI_Send/MPI_Recv calls. This means there are strictly source/target/rest processes related sections. Also, this method
 * can be both non-blocking or blocking. The non-blocking version keeps an internal global request list and must be used in combination with MPIB_Waitall. The blocking version does not and waits for completion.
 *
 * Source process:
 * - tells the spawned processes at the sender/receiver nodes to participate in blocking fashion
 * - it sends around data in a non-blocking way; if one data chunk is of irregular size, it goes to the target process
 *
 * Target process:
 * - blockingly receives a data chunk (potentially of irregular size) from the source process
 * - non-blockingly gets chunks from everyone
 *
 * Spawned processes: blockingly receive and send a chunk of data
 */
int MPIB_scatter_gather_based_p2p(void* sendbuf, int sendcount, int rest,
		MPI_Datatype sendtype, void* recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm, int source, int target, void* interbuf, int MPIB_p2p_blocking) {

	int rank_list[TOTAL_PROC_NUM];
	int rank, size;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	if (rank == source) {
		populate_rank_list(rank_list, source, target, size);
		// The source node signals the non-master processes
		// on the sender and receiver node to participate
		signal_spawned_procs(rank_list, TOTAL_PROC_NUM, 1, source, target, sendcount, rest);
		// Allocate the requests both for scatter and gather operation
		global_reqs_counter +=  TOTAL_PROC_NUM;
		global_reqs = (MPI_Request *) realloc(global_reqs,
				global_reqs_counter * sizeof(MPI_Request));
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(sendtype, &extent);
		char* ptr;
		int start_index = global_reqs_counter - TOTAL_PROC_NUM;
		int i;
		int inc = sendcount * extent;
		for (i = 0, ptr = (char*) sendbuf; i < TOTAL_PROC_NUM; i++) {
			if (rank_list[i] == source) {
				MPI_Aint extent;
				MPI_Type_extent(recvtype, &extent);
				memcpy(interbuf, ptr, recvcount * extent);
				ptr += inc;
				MPI_Isend(interbuf, sendcount, sendtype, target, 0, comm, &global_reqs[start_index+i]);
			} else if (rank_list[i] == target) {
				MPI_Isend(ptr, sendcount + rest, sendtype, rank_list[i], 0,
						comm, &global_reqs[start_index + i]);
				ptr += (rest + sendcount) * extent;
			} else {
				MPI_Isend(ptr, sendcount, sendtype, rank_list[i], 0, comm,
						&global_reqs[start_index + i]);
				ptr += inc;
			}
		}
	}
	else if (rank == target) {
		global_reqs_counter += TOTAL_PROC_NUM;
		global_reqs = (MPI_Request *) realloc(global_reqs,
				global_reqs_counter * sizeof(MPI_Request));
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(recvtype, &extent);
		char* ptr;
		int rank_list[TOTAL_PROC_NUM];
		populate_rank_list(rank_list, source, target, size);
		int start_index = global_reqs_counter - TOTAL_PROC_NUM;
		int i;
		for (i = 0, ptr = (char*) recvbuf; i < TOTAL_PROC_NUM; i++) {
			if (rank_list[i] == target) {
				MPI_Irecv(ptr, (recvcount + rest), recvtype, source, 0, comm,
					&global_reqs[start_index+i]);
				//global_reqs[start_index+i] = MPI_REQUEST_NULL;
				ptr += (recvcount + rest) * extent;
			}
			else {
				MPI_Irecv(ptr, recvcount, recvtype, rank_list[i], 0, comm,
						&global_reqs[start_index+i]);
				ptr += recvcount * extent;
			}
		}
	}

	if ((rank == source ) || (rank == target)) {
		// At this point, a blocking call completes while a non-blocking call will
		// be delayed until the wrapper Waitall function is called
		if (MPIB_p2p_blocking) {
			MPI_Waitall(global_reqs_counter, global_reqs, MPI_STATUSES_IGNORE);
			free(global_reqs);
			global_reqs = NULL;
			global_reqs_counter = 0;
		}
	}
	// All spawned processes simply forward chunks of data
	else {
		MPI_Recv(interbuf, recvcount, recvtype, source, 0, comm,
				MPI_STATUS_IGNORE);
		MPI_Send(interbuf, sendcount, sendtype, target, 0, comm);
	}

	return MPI_SUCCESS;
}

/*!
 * Allocate temporary buffer, call modified point-to-point and
 * release the buffer
 */
int MPIB_Send_sg(void* buf, int count, MPI_Datatype datatype, int dest,
		int tag, MPI_Comm comm, int MPIB_p2p_blocking) {
	int size, rank;
	//this process should never receive final data, receive buffer is NULL
	char *empty = NULL;
	char* intermed_array;
	MPI_Comm_size(intracomm, &size);
	MPI_Comm_rank(intracomm, &rank);
	int diff = count - (count / TOTAL_PROC_NUM) * TOTAL_PROC_NUM;
	if (rank == dest)
		intermed_array = (char *) malloc(
				((count / TOTAL_PROC_NUM) + diff) * sizeof(char));
	else
		intermed_array = (char *) malloc(
				(count / TOTAL_PROC_NUM) * sizeof(char));
	MPIB_scatter_gather_based_p2p(buf, (count / TOTAL_PROC_NUM), diff, datatype,
			empty, (count / TOTAL_PROC_NUM), datatype, -1, intracomm,
			rank, dest, intermed_array, MPIB_p2p_blocking);
	free(intermed_array);
	return 0;
}

/*!
 * Allocate temporary buffer, call modified point-to-point and
 * release the buffer
 */
int MPIB_Recv_sg(void* buf, int count, MPI_Datatype datatype, int source,
		int tag, MPI_Comm comm, MPI_Status* status, int MPIB_p2p_blocking) {
	int size, rank;
	char *empty = NULL;
	char *intermed_array;
	MPI_Comm_size(intracomm, &size);
	MPI_Comm_rank(intracomm, &rank);
	int diff = count - (count / TOTAL_PROC_NUM) * TOTAL_PROC_NUM;
	if (rank != source) // which means = target
		intermed_array = (char *) malloc(
				((count / TOTAL_PROC_NUM) + diff) * sizeof(char));
	else
		intermed_array = (char *) malloc(
				(count / TOTAL_PROC_NUM) * sizeof(char));
	MPIB_scatter_gather_based_p2p(empty, (count / TOTAL_PROC_NUM), diff, datatype,
			buf, (count / TOTAL_PROC_NUM), datatype, -1, intracomm,
			source, rank, intermed_array, MPIB_p2p_blocking);
	free(intermed_array);
	return 0;
}
