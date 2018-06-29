#include "libmpib_coll.h"
#include <mpi.h>
#include <string.h>
#include <malloc.h>
#include "p2p/mpib_p2p.h"

#define MIN(x,y) (x<y)?x:y

int MPIB_Scatter_flat_nb(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(sendtype, &extent);
		int inc = sendcount * extent;
		int i;
		char* ptr;
		MPI_Request req[size];
		for (i = 0, ptr = (char*)sendbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				MPI_Aint extent;
				MPI_Type_extent(recvtype, &extent);
				memcpy(recvbuf, ptr, recvcount * extent);
				req[i] = MPI_REQUEST_NULL;
			} else
				MPIB_Isend(ptr, sendcount, sendtype, i, 0, comm, &req[i]);
		}
		MPIB_Waitall(size, req, MPI_STATUSES_IGNORE);
	} else
		MPIB_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
	return MPI_SUCCESS;
}

/*
int MPIB_Scatter_flat(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(sendtype, &extent);
		int inc = sendcount * extent;
		int i;
		char* ptr;
		MPI_Request req[size];
		for (i = 0, ptr = (char*)sendbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				MPI_Aint extent;
				MPI_Type_extent(recvtype, &extent);
				memcpy(recvbuf, ptr, recvcount * extent);
				req[i] = MPI_REQUEST_NULL;
			} else
				MPIB_Send_sg(ptr, sendcount, sendtype, i, 0, comm, &req[i]);
		}
		MPIB_Waitall(size, req, MPI_STATUSES_IGNORE);
	} else
		MPIB_Recv_sg(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
	return MPI_SUCCESS;
}
*/

int MPIB_Scatter_flat_rsend(void* sendbuf, int sendcount,
		MPI_Datatype sendtype, void* recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm) {
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(sendtype, &extent);
		int inc = sendcount * extent;
		int i;
		char* ptr;
		MPI_Barrier(comm);
		for (i = 0, ptr = (char*) sendbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				MPI_Aint extent;
				MPI_Type_extent(recvtype, &extent);
				memcpy(recvbuf, ptr, recvcount * extent);
			} else {
				MPI_Rsend(ptr, sendcount, sendtype, i, 0, comm);
			}

		}
	} else {
		MPI_Request req;
		MPI_Irecv(recvbuf, recvcount, recvtype, root, 0, comm, &req);
		MPI_Barrier(comm);
		MPI_Wait(&req, MPI_STATUS_IGNORE);
	}
	return MPI_SUCCESS;
}


int MPIB_Gather_flat_nb(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(recvtype, &extent);
		int inc = recvcount * extent;
		int i;
		char* ptr;
		MPI_Request req[size];
		for (i = 0, ptr = (char*)recvbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				memcpy(ptr, sendbuf, inc);
				req[i] = MPI_REQUEST_NULL;
			}
			else
				MPIB_Irecv(ptr, recvcount, recvtype, i, 0, comm, &req[i]);
		}
		MPIB_Waitall(size, req, MPI_STATUSES_IGNORE);
	} else
		MPIB_Send(sendbuf, sendcount, sendtype, root, 0, comm);
	return MPI_SUCCESS;
}

/*
int MPIB_Gather_flat_with_mod_p2p(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(recvtype, &extent);
		int inc = recvcount * extent;
		int i;
		char* ptr;
		MPI_Request req[size];
		for (i = 0, ptr = (char*)recvbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				memcpy(ptr, sendbuf, inc);
				req[i] = MPI_REQUEST_NULL;
			}
			else
				MPIB_Recv(ptr, recvcount, recvtype, i, 0, comm, &req[i]);
		}
	} else
		MPIB_Send_sg(sendbuf, sendcount, sendtype, root, 0, comm);
	return MPI_SUCCESS;
}
 */

int MPIB_Gather_flat_rsend(void* sendbuf, int sendcount, MPI_Datatype sendtype,
		void* recvbuf, int recvcount, MPI_Datatype recvtype, int root,
		MPI_Comm comm) {
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		MPI_Request* reqs;
		int size;
		MPI_Comm_size(comm, &size);
		MPI_Aint extent;
		MPI_Type_extent(recvtype, &extent);
		int inc = recvcount * extent;
		int i;
		char* ptr;
		reqs = (MPI_Request *) malloc(size * sizeof(MPI_Request));
		for (i = 0, ptr = (char*) recvbuf; i < size; i++, ptr += inc) {
			if (i == root) {
				memcpy(ptr, sendbuf, inc);
				reqs[i] = MPI_REQUEST_NULL;
			} else
				MPI_Irecv(ptr, recvcount, recvtype, i, 0, comm, &reqs[i]);
		}
		MPI_Barrier(comm);
		MPI_Waitall(size, reqs, MPI_STATUSES_IGNORE);
		free(reqs);
	} else {
		MPI_Barrier(comm);
		//eager protocol send
		MPI_Rsend(sendbuf, sendcount, sendtype, root, 0, comm);
	}
	return MPI_SUCCESS;
}

int MPIB_Gather_flat_sync(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank, size;
	int i;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &size);
	int first_seg_count = MIN(32768, sendcount);
	MPI_Status status;
	//size_t typelng;
	MPI_Aint extent;
	char *ptmp;
	MPI_Request first_seg_req;
	
	if (rank == root) {
		MPI_Type_extent(recvtype, &extent);	
		MPI_Request *reqs;
		reqs = (MPI_Request *) malloc(size * sizeof(MPI_Request));
		for (i = 0; i < size; i++) {
			if (i == rank) {
				reqs[i] = MPI_REQUEST_NULL;
				continue;
			}
			ptmp = (char*) recvbuf + i * recvcount * extent;
			MPI_Irecv(ptmp, first_seg_count, recvtype, i, 0, comm, &first_seg_req);
			MPI_Send(sendbuf, 0, MPI_BYTE, i, 0, comm);

	
			if (first_seg_count < sendcount) {
				ptmp = (char*) recvbuf + (i * recvcount + first_seg_count) * extent;
				MPI_Irecv(ptmp, (recvcount - first_seg_count),  recvtype, i, 0, comm, reqs+i);
			}
			MPI_Wait(&first_seg_req, MPI_STATUS_IGNORE);
		}
		
		if (MPI_IN_PLACE != sendbuf) {
			memcpy(recvbuf + rank * recvcount * extent, sendbuf, recvcount * extent);
			//following line causes crash
			//MPI_Sendrecv(sendbuf, sendcount, sendtype, rank, 0, (char*) recvbuf + rank * recvcount * extent, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
		}
		
		if (first_seg_count < sendcount) {
			MPI_Waitall(size, reqs, MPI_STATUSES_IGNORE);
		}
		free(reqs);
	} else {
		MPI_Recv(recvbuf, 0, MPI_BYTE, root, 0, comm, &status);
		MPI_Send(sendbuf, first_seg_count, sendtype, root, 0, comm );
		//there is more to send
		if (first_seg_count < sendcount) {
			//MPI_Type_size(sendtype, &typelng);
			MPI_Type_extent(sendtype,  &extent);
			MPI_Send(sendbuf+extent*first_seg_count, (sendcount - first_seg_count), sendtype, root, 0, comm);
		}
	}

	return MPI_SUCCESS;
}
