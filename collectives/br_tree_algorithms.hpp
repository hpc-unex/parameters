#ifndef BR_TREE_ALGORITHMS_HPP_
#define BR_TREE_ALGORITHMS_HPP_

#include "libmpib_coll.h"
#include "mpib_coll.h"
#include "comm_tree.hpp"
#include <boost/graph/graphviz.hpp>

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of bcast
 */
template <typename Builder>
int MPIB_Bcast_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
	using namespace MPIB::comm_tree;
	// build graph
	int size;
	MPI_Comm_size(comm, &size);
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Aint extent;
	MPI_Type_extent(datatype, &extent);
	Graph graph;
	Vertex r, u, v;
	builder.build(size, root, rank, count * extent, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph);
	// recv from parent
	if (rank != root)
		MPI_Recv(buffer, count, datatype, get(vertex_index, graph, u), 0, comm, MPI_STATUS_IGNORE);
	// send to children - start from the last
	MPI_Request* reqs = (MPI_Request*)malloc(sizeof(MPI_Request) * size);
	int child_counter;
	Adjacency_iterator ai, ai_end;
	tie(ai, ai_end) = adjacent_vertices(v, graph);
	for(child_counter = 0; ai != ai_end; child_counter++)
		MPI_Isend(buffer, count, datatype, get(vertex_index, graph, (order == R2L) ? *(--ai_end) : *(ai++)), 0, comm, &reqs[child_counter]);
	MPI_Waitall(child_counter, reqs, MPI_STATUSES_IGNORE);
	free(reqs);
	return MPI_SUCCESS;
}

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of reduce.
 * \note Does not perform MPI operations but allocates memory for subproduct.
 * MPI internals should be used. For example, Open MPI:
 * \code
 * #ifdef HAVE_OPENMPI_OMPI_OP_OP_H
 * #include <openmpi/ompi/op/op.h>
 * #endif
 * ...
 * int MPIB_Reduce_tree_algorithm(...) {
 * 	...
 * #ifdef HAVE_OPENMPI_OMPI_OP_OP_H
 * 	ompi_op_reduce(op, buffer, sendbuf, count, datatype);
 * #endif
 * 	...
 * }
 * \endcode
 * TODO: implement MPI operation in the reduce tree algorithm
 */
template <typename Builder>
int MPIB_Reduce_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
	MPI_Op op, int root, MPI_Comm comm)
{
	using namespace MPIB::comm_tree;
	// build graph
	int size;
	MPI_Comm_size(comm, &size);
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Aint extent;
	MPI_Type_extent(datatype, &extent);
	Graph graph;
	Vertex r, u, v;
	builder.build(size, root, rank, count * extent, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph);
	// subproduct for MPI operation
	void* buffer = rank == root ? recvbuf : malloc(count * extent);
	// receive from children - start from the first
	MPI_Request* reqs = (MPI_Request*)malloc(sizeof(MPI_Request) * size);
	int child_counter;
	Adjacency_iterator ai, ai_end;
	tie(ai, ai_end) = adjacent_vertices(v, graph);
	for(child_counter = 0; ai != ai_end; child_counter++) {
		MPI_Irecv(buffer, count, datatype, get(vertex_index, graph, (order == R2L) ? *(--ai_end) : *(ai++)), 0, comm, &reqs[child_counter]);
		// MPI operation to be here: buffer op= sendbuf.
	}
	MPI_Waitall(child_counter, reqs, MPI_STATUSES_IGNORE);
	free(reqs);
	// send to parent
	if (rank != root) {
		MPI_Send(buffer, count, datatype, get(vertex_index, graph, u), 0, comm);
		free(buffer);
	}
	return MPI_SUCCESS;
}

#endif /*BR_TREE_ALGORITHMS_HPP_*/
