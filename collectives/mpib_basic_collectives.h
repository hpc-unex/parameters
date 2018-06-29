#ifndef MPIB_BASIC_COLLECTIVES_H_
#define MPIB_BASIC_COLLECTIVES_H_

#include <mpi.h>

/*!
 * \defgroup basic_collectives Basic algorithms of MPI collective operations
 * This module provides basic, mostly flat-tree, algorithms of MPI collective operations.
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*! Flat-tree scatter using non-blocking standard or modified p2p*/
int MPIB_Scatter_flat_nb(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree gather using standard or modified p2p*/
int MPIB_Gather_flat_nb(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree scatter using MPI_Rsend => eager protocol*/
int MPIB_Scatter_flat_rsend(void* sendbuf, int sendcount, MPI_Datatype sendtype,
        void* recvbuf, int recvcount, MPI_Datatype recvtype,
        int root, MPI_Comm comm);

/*! Flat-tree gather using MPI_Rsend => eager protocol*/
int MPIB_Gather_flat_rsend(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree gatherv with sync, taken from OMPI trunk*/
int MPIB_Gatherv_flat_sync(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree scatterv */
int MPIB_Scatterv_flat(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree gatherv */
int MPIB_Gatherv_flat(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree scatterv with sendcounts sorted in ascending order */
int MPIB_Scatterv_sorted_flat_asc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree gatherv with recvcounts sorted in ascending order */
int MPIB_Gatherv_sorted_flat_asc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree scatterv with sendcounts sorted in descending order */
int MPIB_Scatterv_sorted_flat_dsc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Flat-tree gatherv with recvcounts sorted in descending order */
int MPIB_Gatherv_sorted_flat_dsc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

#ifdef __cplusplus
}
#endif
/*! \} */

#endif /*MPIB_BASIC_COLLECTIVES_H_*/
