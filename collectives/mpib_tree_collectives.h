#ifndef MPIB_TREE_COLLECTIVES_H_
#define MPIB_TREE_COLLECTIVES_H_

#include <mpi.h>

/*!
 * \defgroup tree_collectives Tree-based algorithms of MPI collective operations
 * This module provides tree-based algorithms of MPI collective operations.
 * For rapid implementation, the Graph Boost C++ library is used.
 * The sources of tree algorithms of collectives must be compiled by C++.
 * 
 * A tree-based algorithm of a collective operation \c X consists of the following components:
 * - <strong>Communication tree</strong> \ref MPIB::comm_tree
 * - <strong>Function template for the tree-based algorithm</strong> \c MPIB_X_tree_algorithm
 * (described in \ref tree_algorithms)
 * - <strong>Communication tree builder</strong> \c Y_builder, a C++ class that builds a communication tree.
 * (described in \ref MPIB::BRSG and \ref MPIB::SGv)
 *
 * The tree-based implementation looks as follows:
 * \code
extern "C" int MPIB_X_Y(standard args) {
	return MPIB_X_tree_algorithm(Y_builder(), standard args);
}
 * \endcode
 * For example, \ref MPIB_Scatter_binomial, a binomial scatter.
 * 
 * This approach provides flexibility:
 * - Tree-based algorithm of the collective operation is a basis for implementation of algorithms
 * with communication trees of different shapes (for example, binomial and binary algorithms of scatter
 * use the general tree-based algorithm of scatter).
 * - Communication tree is a universal interface between tree-based algorithms and tree builders.
 * It can be reused for different communication operations (for example, scatter/gather and bcast/reduce
 * use the same simple communication tree).
 * - Tree and communication tree builders can be reused per tree shape (for example,
 * the same binomial communication tree builder is used in binomial scatter/gather and bcast/reduce).
 *
 * \dotfile collectives/tree_collectives.dot
 * 
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*! Binomial Bcast */
int MPIB_Bcast_binomial(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm);

/*! Binomial Reduce */
int MPIB_Reduce_binomial(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
	MPI_Op op, int root, MPI_Comm comm);

/*! Binomial Scatter */
int MPIB_Scatter_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	int root, MPI_Comm comm);

/*! Binomial Gather */
int MPIB_Gather_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial scatterv */
int MPIB_Scatterv_binomial(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial gatherv. */
int MPIB_Gatherv_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial scatterv with children sorted by counts in ascending order. */
int MPIB_Scatterv_sorted_binomial_asc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial gatherv with children sorted by counts in ascending order. */
int MPIB_Gatherv_sorted_binomial_asc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial scatterv with children sorted by counts in descending order. */
int MPIB_Scatterv_sorted_binomial_dsc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Binomial gatherv with children sorted by counts in descending order. */
int MPIB_Gatherv_sorted_binomial_dsc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Traff algorithm of Scatterv. Based on \latexonly\cite{Traff2004}\endlatexonly. */
int MPIB_Scatterv_Traff(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Traff algorithm of Gatherv. Based on \latexonly\cite{Traff2004}\endlatexonly. */
int MPIB_Gatherv_Traff(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

#ifdef __cplusplus
}
#endif
/*! \} */

#endif /*MPIB_TREE_COLLECTIVES_H_*/
