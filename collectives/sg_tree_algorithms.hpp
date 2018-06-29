#ifndef SG_TREE_ALGORITHMS_HPP_
#define SG_TREE_ALGORITHMS_HPP_

#include "libmpib_coll.h"
#include "comm_tree.hpp"
#include <boost/graph/graphviz.hpp>

namespace MPIB {
/*! Auxiliaries for scatter/gather tree algorithms */
namespace SG {
using namespace comm_tree;

/*!
 * Finds the data to send (scatter) or recv (gather) from the local buffer (rank2index).
 * Default copy constructors are used. Arrays must be preallocated (max = comm size).
 */
class Assembler {
private:
	/*! local rank2index */
	const map<int, int>& rank2index;
	/*! recv/send count */
	const int rscount;
	/*! number of data blocks to send */
	int& count;
	/*! lengths to send/recv */
	int* lengths;
	/*! indices to send/recv */
	int* indices;
public:
	Assembler(const map<int, int>& _rank2index, const int _rscount,
			int& _count, int* _lengths, int* _indices):
				rank2index(_rank2index), rscount(_rscount),
				count(_count), lengths(_lengths), indices(_indices) {
		count = 0;
	}
	void preorder(Vertex vertex, Tree& tree) {
		lengths[count] = rscount;
		indices[count] = rscount * rank2index.find(get(vertex_index, tree, vertex))->second;
		count++;
	}
	void inorder(Vertex vertex, Tree& tree) {}
	void postorder(Vertex vertex, Tree& tree) {}
};

/*!
 * Builds the rank2index for the local buffer. Default copy constructors are used.
 */
class Indexer {
private:
	/*! index (for internal use) */
	int& index;
	/*! local rank2index */
	map<int, int>& rank2index;
public:
	Indexer(int& _index, map<int, int>& _rank2index):
		index(_index), rank2index(_rank2index) {
		index = 0;
	}
	void preorder(Vertex vertex, Tree& tree) {
		rank2index[get(vertex_index, tree, vertex)] = index++;
	}
	void inorder(Vertex vertex, Tree& tree) {}
	void postorder(Vertex vertex, Tree& tree) {}
};

/*! Edge writer */
class Edge_writer {
private:
	Graph& graph;
	const int rscount;

	/*! Returns a total count to send/recv */
	class Indexer {
	private:
		const int rscount;
		int& count;
	public:
		Indexer(const int rscount, int& _count): rscount(rscount), count(_count) {
			count = 0;
		}
		void preorder(Vertex vertex, Tree& tree) {
			count += rscount;
		}
		void inorder(Vertex vertex, Tree& tree) {}
		void postorder(Vertex vertex, Tree& tree) {}
	};

public:
	Edge_writer(Graph& _graph, const int _rscount):
		graph(_graph), rscount(_rscount) {}
	void operator()(std::ostream& out, const Edge& e) const {
		Vertex v = target(e, graph);
		Tree tree = Tree(graph, v);
		int count;
		traverse_tree(v, tree, Indexer(rscount, count));
		out << "[label = \"" << count << "\"]";
	}
};
}
}

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of scatter
 */
template <typename Builder>
int MPIB_Scatter_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype, 
	int root, MPI_Comm comm)
{
	using namespace MPIB::SG;
	// build graph
	int size;
	MPI_Comm_size(comm, &size);
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Aint recvext;
	MPI_Type_extent(recvtype, &recvext);
	Graph graph;
	Vertex r, u, v;
	builder.build(size, root, rank, recvcount * recvext, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph, default_writer(), Edge_writer(graph, recvcount * recvext));
	// tree without parent map for traverse
	Tree tree = Tree(graph, r);
	// local local buffer based on rank2index
	map<int, int> rank2index;
	char* buffer = NULL;
	if (rank == root) {
		// prepare local buffer
		for (int i = 0; i < size; i++)
			rank2index[i] = i;
		buffer = (char*)sendbuf;
	} else {
		// prepare local buffer
		int index;
		traverse_tree(v, tree, Indexer(index, rank2index));
		int count = rank2index.size() * recvcount;
		buffer = (char*)malloc(sizeof(char) * count * recvext);
		// recv data from parent
		MPI_Recv(buffer, count, recvtype, get(vertex_index, graph, u), 0, comm, MPI_STATUS_IGNORE);
	}
	// copy own data
	memcpy(recvbuf, buffer + rank2index[rank] * recvcount * recvext, recvcount * recvext);
	// children's data
	int subtree_size = rank2index.size() - 1;
	int count;
	int* lengths = (int*)malloc(sizeof(int) * subtree_size);
	int* indices = (int*)malloc(sizeof(int) * subtree_size);
	MPI_Datatype* dts = (MPI_Datatype*)malloc(sizeof(MPI_Datatype) * subtree_size);
	MPI_Request* reqs = (MPI_Request*)malloc(sizeof(MPI_Request) * subtree_size);
	int child_counter;
	Adjacency_iterator ai, ai_end;
	tie(ai, ai_end) = adjacent_vertices(v, graph);
	for(child_counter = 0; ai != ai_end; child_counter++) {
		Vertex t = (order == R2L) ? *(--ai_end) : *(ai++);
		// find data to send
		traverse_tree(t, tree,
			Assembler(rank2index, recvcount, count, lengths, indices));
		// send data to child
		MPI_Type_indexed(count, lengths, indices, recvtype, &dts[child_counter]);
		MPI_Type_commit(&dts[child_counter]);
		MPI_Isend(buffer, 1, dts[child_counter], get(vertex_index, graph, t), 0, comm, &reqs[child_counter]);
	}
	free(lengths);
	free(indices);
	MPI_Waitall(child_counter, reqs, MPI_STATUSES_IGNORE);
	free(reqs);
	for (int i = 0; i < child_counter; i++)
		MPI_Type_free(&dts[i]);
	free(dts);
	if (rank != root)
		free(buffer);
	return MPI_SUCCESS;
}

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of gather
 */
template <typename Builder>
int MPIB_Gather_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* sendbuf, int sendcount, MPI_Datatype sendtype, 
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	using namespace MPIB::SG;
	// build graph
	int size;
	MPI_Comm_size(comm, &size);
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Aint sendext;
	MPI_Type_extent(sendtype, &sendext);
	Graph graph;
	Vertex r, u, v;
	builder.build(size, root, rank, sendcount * sendext, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph, default_writer(), Edge_writer(graph, sendcount * sendext));
	// tree without parent map for traverse
	Tree tree = Tree(graph, r);
	// local local buffer based on rank2index
	map<int, int> rank2index;
	char* buffer = NULL;
	if (rank == root) {
		// prepare local buffer
		for (int i = 0; i < size; i++)
			rank2index[i] = i;
		buffer = (char*)recvbuf;
	} else {
		// prepare local buffer
		int index;
		traverse_tree(v, tree, Indexer(index, rank2index));
		buffer = (char*)malloc(sizeof(char) * rank2index.size() * sendcount * sendext);
	}
	// copy own data
	memcpy(buffer + rank2index[rank] * sendcount * sendext, sendbuf, sendcount * sendext);
	// children's data
	int subtree_size = rank2index.size() - 1;
	int count;
	int* lengths = (int*)malloc(sizeof(int) * subtree_size);
	int* indices = (int*)malloc(sizeof(int) * subtree_size);
	MPI_Datatype* dts = (MPI_Datatype*)malloc(sizeof(MPI_Datatype) * subtree_size);
	MPI_Request* reqs = (MPI_Request*)malloc(sizeof(MPI_Request) * subtree_size);
	int child_counter;
	Adjacency_iterator ai, ai_end;
	tie(ai, ai_end) = adjacent_vertices(v, graph);
	for(child_counter = 0; ai != ai_end; child_counter++) {
		Vertex t = (order == R2L) ? *(--ai_end) : *(ai++);
		// find data to recv
		traverse_tree(t, tree,
			Assembler(rank2index, sendcount, count, lengths, indices));
		// recv data from child
		MPI_Type_indexed(count, lengths, indices, sendtype, &dts[child_counter]);
		MPI_Type_commit(&dts[child_counter]);
		MPI_Irecv(buffer, 1, dts[child_counter], get(vertex_index, graph, t), 0, comm, &reqs[child_counter]);
	}
	free(lengths);
	free(indices);
	MPI_Waitall(child_counter, reqs, MPI_STATUSES_IGNORE);
	free(reqs);
	for (int i = 0; i < child_counter; i++)
		MPI_Type_free(&dts[i]);
	free(dts);
	// send data to parent
	if (rank != root)
		MPI_Send(buffer, rank2index.size() * sendcount, sendtype, get(vertex_index, graph, u), 0, comm);
	if (rank != root)
		free(buffer);
	return MPI_SUCCESS;
}

#endif /*SG_TREE_ALGORITHMS_HPP_*/
