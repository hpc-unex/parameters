#ifndef MPIB_COLLECTIVES_H_
#define MPIB_COLLECTIVES_H_

#include <mpi.h>

/*!
 * \defgroup defs Definitions of MPI communication operations
 * This module provides the definitions of MPI communication operations.
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*! Scatter typedef */
typedef int (*MPIB_Scatter)(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	int root, MPI_Comm comm);

/*! Gather typedef */
typedef int (*MPIB_Gather)(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Scatterv typedef */
typedef int (*MPIB_Scatterv)(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Gatherv typedef */
typedef int (*MPIB_Gatherv)(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Bcast typedef */
typedef int (*MPIB_Bcast)(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

/*! Reduce typedef */
typedef int (*MPIB_Reduce)(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
	MPI_Op op, int root, MPI_Comm comm);

#ifdef __cplusplus
}
#endif
/*! \} */

#endif /*MPIB_COLLECTIVES_H_*/
