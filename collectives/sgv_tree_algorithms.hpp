#ifndef SGV_TREE_ALGORITHMS_HPP_
#define SGV_TREE_ALGORITHMS_HPP_

#include "libmpib_coll.h"
#include "comm_tree.hpp"
#include "mpib_coll.h"
#include <boost/graph/graphviz.hpp>
#include <limits.h>

namespace MPIB {

/*! Auxiliaries for scatterv/gatherv tree algorithms */
namespace SGv {
using namespace comm_tree;

/*! tag for scatterv/gatherv - not safe, incremental on all processors */
int tag = 0;

/*!
 * Finds the data to send (scatterv) or recv (gatherv) from the local buffer (rank2index, counts, displs).
 * Default copy constructors are used. Arrays must be preallocated (max = comm size).
 */
class Assembler {
private:
	/*! local buffer rank2index */
	const map<int, int>& rank2index;
	/*! local buffer counts */
	const int* counts;
	/*! local buffer displs */
	const int* displs;
	/*! number of data blocks to send */
	int& count;
	/*! lengths to send/recv accordingly to the buffer counts */
	int* lengths;
	/*! indices to send/recv accordingly to the buffer displs */
	int* indices;
public:
	Assembler(const map<int, int>& _rank2index, const int* _counts, const int* _displs,
		int& _count, int* _lengths, int* _indices):
		rank2index(_rank2index), counts(_counts), displs(_displs),
		count(_count), lengths(_lengths), indices(_indices)
	{
		count = 0;
	}
	void preorder(Vertex vertex, Tree& tree) {
		int index = rank2index.find(get(vertex_index, tree, vertex))->second;
		lengths[count] = counts[index];
		indices[count] = displs[index];
		count++;
	}
	void inorder(Vertex vertex, Tree& tree) {}
	void postorder(Vertex vertex, Tree& tree) {}
};

/*!
 * Builds the rank2index, counts and displs for the local buffer.
 * Default copy constructors are used. Arrays must be preallocated (max = comm size).
 */
class Indexer {
private:
	/*! global counts */
	const int* rscounts;
	/*! index (for internal use) */
	int& index;
	/*! rank-to-index in the buffer */
	map<int, int>& rank2index;
	/*! total count to recv/send */
	int* counts;
	/*! displs in the buffer to recv/send */
	int* displs;
	int& count;
	/*! counts in the buffer to recv/send */
public:
	Indexer(const int* _rscounts, int& _index, map<int, int>& _rank2index, int* _counts, int* _displs, int& _count):
		rscounts(_rscounts), index(_index), rank2index(_rank2index), counts(_counts), displs(_displs), count(_count)
	{
		index = 0;
		count = 0;
	}
	void preorder(Vertex vertex, Tree& tree) {
		int rank = get(vertex_index, tree, vertex);
		rank2index[rank] = index;
		counts[index] = rscounts[rank];
		displs[index] = index == 0 ? 0 : displs[index - 1] + counts[index - 1];
		count += counts[index];
		index++;
	}
	void inorder(Vertex vertex, Tree& tree) {}
	void postorder(Vertex vertex, Tree& tree) {}
};

/*! Vertex writer */
class Vertex_writer {
private:
	Graph& graph;
	const int* counts;
public:
	Vertex_writer(Graph& _graph, const int* _counts): graph(_graph), counts(_counts) {}
	void operator()(std::ostream& out, const Vertex& v) const {
		int rank = get(vertex_index, graph, v);
		out << "[label = \"" << rank << " (" << counts[rank] << ")\"]";
	}
};

/*! Edge writer */
class Edge_writer {
private:
	Graph& graph;
	const int* counts;

	/*! Returns a total count to send/recv */
	class Indexer {
	private:
		const int* counts;
		int& count;
	public:
		Indexer(const int* _counts, int& _count): counts(_counts), count(_count) {
			count = 0;
		}
		void preorder(Vertex vertex, Tree& tree) {
			count += counts[get(vertex_index, tree, vertex)];
		}
		void inorder(Vertex vertex, Tree& tree) {}
		void postorder(Vertex vertex, Tree& tree) {}
	};

public:
	Edge_writer(Graph& _graph, const int* _counts):
		graph(_graph), counts(_counts) {}
	void operator()(std::ostream& out, const Edge& e) const {
		Vertex v = target(e, graph);
		int count;
		Tree tree(graph, v);
		traverse_tree(v, tree, Indexer(counts, count));
		out << "[label = \"" << count << "\"]";
	}
};
}
}

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of scatterv
 */
template <typename Builder>
int MPIB_Scatterv_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm _comm)
{
	using namespace MPIB::SGv;
	// communicator
	MPI_Comm comm = _comm;
	if (mpib_coll_sgv == 0)
		MPI_Comm_dup(_comm, &comm);
	// tag
	int tag = 0;
	if (mpib_coll_sgv == 2) {
		if (++tag == INT_MAX)
			tag = 0;
		tag = tag;
	}
	// counts
	int size;
	MPI_Comm_size(comm, &size);
	int* counts = (int*)malloc(sizeof(int) * size);
	MPI_Aint sendext;
	MPI_Type_extent(sendtype, &sendext);
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root || mpib_coll_sgv == 3) {
		for (int i = 0; i < size; i++)
			counts[i] = sendcounts[i] * sendext;
	}
	if (mpib_coll_sgv == 1)
		MPI_Bcast(counts, size, MPI_INT, root, comm);
	// build graph everywhere (except for propagated modes)
	Graph graph;
	Vertex r, u, v;
	if (rank == root || mpib_coll_sgv == 1 || mpib_coll_sgv == 3)
		builder.build(size, root, rank, counts, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph, Vertex_writer(graph, counts), Edge_writer(graph, counts));
	// local buffer, counts, displs based on rank2index
	map<int, int> rank2index;
	char* buffer = NULL;
	int* _counts = (int*)malloc(sizeof(int) * size);
	int* _displs = (int*)malloc(sizeof(int) * size);
	if (rank == root) {
		// prepare local buffer, counts, displs
		for (int i = 0; i < size; i++) {
			rank2index[i] = i;
			_counts[i] = sendcounts[i] * sendext;
			_displs[i] = displs[i] * sendext;
		}
		buffer = (char*)sendbuf;
	} else {
		// recv data and counts from parent (propagated modes)
		if (mpib_coll_sgv == 0 || mpib_coll_sgv == 2) {
			MPI_Status status;
			MPI_Probe(MPI_ANY_SOURCE, tag, comm, &status);
			int length;
			MPI_Get_count(&status, MPI_CHAR, &length);
			buffer = (char*)malloc(sizeof(char) * (length - sizeof(int) * size));
			MPI_Datatype dt;
			int lengths[2] = {length - sizeof(int) * size, size};
			MPI_Aint indices[2] = {0, (char*)counts - buffer};
			MPI_Datatype types[2] = {MPI_CHAR, MPI_INT};
			MPI_Type_struct(2, lengths, indices, types, &dt);
			MPI_Type_commit(&dt);
			MPI_Recv(buffer, 1, dt, status.MPI_SOURCE, status.MPI_TAG, comm, MPI_STATUS_IGNORE);
			MPI_Type_free(&dt);
			builder.build(size, root, rank, counts, graph, r, u, v);
		}
		// prepare local buffer
		Tree tree = Tree(graph, r);
		int index;
		int count;
		traverse_tree(v, tree,
			Indexer(counts, index, rank2index, _counts, _displs, count));
		// recv data from parent (except for propagated modes)
		if (mpib_coll_sgv == 1 || mpib_coll_sgv == 3) {
			buffer = (char*)malloc(sizeof(char) * count);
			MPI_Recv(buffer, count, MPI_CHAR, get(vertex_index, graph, u), 0, comm, MPI_STATUS_IGNORE);
		}
	}
	// copy own data
	MPI_Aint recvext;
	MPI_Type_extent(recvtype, &recvext);
	memcpy(recvbuf, buffer + _displs[rank2index[rank]], recvcount * recvext);
	// children's data
	Tree tree = Tree(graph, r);
	int subtree_size = rank2index.size(); // we need an extra element for the propagated modes
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
			Assembler(rank2index, _counts, _displs, count, lengths, indices));
		// data + counts (propagated modes)
		if (mpib_coll_sgv == 0 || mpib_coll_sgv == 2) {
			lengths[count] = sizeof(int) * size;
			indices[count] = (char*)counts - buffer;
			count++;
		}
		// send data to child
		MPI_Type_indexed(count, lengths, indices, MPI_CHAR, &dts[child_counter]);
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
	free(_counts);
	free(_displs);
	free(counts);
	if (mpib_coll_sgv == 0)
		MPI_Comm_free(&comm);
	return MPI_SUCCESS;
}

/*!
 * \ingroup tree_algorithms
 * Base tree algorithm of gatherv
 */
template <typename Builder>
int MPIB_Gatherv_tree_algorithm(Builder builder, MPIB_child_traverse_order order,
	void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm _comm)
{
	using namespace MPIB::SGv;
	// communicator
	MPI_Comm comm = _comm;
	if (mpib_coll_sgv == 0)
		MPI_Comm_dup(_comm, &comm);
	// tag
	int tag = 0;
	if (mpib_coll_sgv == 2) {
		if (++tag == INT_MAX)
			tag = 0;
		tag = tag;
	}
	// counts
	int size;
	MPI_Comm_size(comm, &size);
	int* counts = (int*)malloc(sizeof(int) * size);
	MPI_Aint recvext;
	MPI_Type_extent(recvtype, &recvext);
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root || mpib_coll_sgv == 3) {
		for (int i = 0; i < size; i++)
			counts[i] = recvcounts[i] * recvext;
	}
	if (mpib_coll_sgv == 1)
		MPI_Bcast(counts, size, MPI_INT, root, comm);
	// build graph everywhere (except for propagated modes)
	Graph graph;
	Vertex r, u, v;
	if (rank == root || mpib_coll_sgv == 1 || mpib_coll_sgv == 3)
		builder.build(size, root, rank, counts, graph, r, u, v);
	if ((rank == root) && mpib_coll_verbose)
		write_graphviz(cout, graph, Vertex_writer(graph, counts), Edge_writer(graph, counts));
	// local buffer, counts, displs based on rank2index
	map<int, int> rank2index;
	char* buffer = NULL;
	int count;
	int* _counts = (int*)malloc(sizeof(int) * size);
	int* _displs = (int*)malloc(sizeof(int) * size);
	if (rank == root) {
		// prepare local buffer, counts, displs
		for (int i = 0; i < size; i++) {
			rank2index[i] = i;
			_counts[i] = recvcounts[i] * recvext;
			_displs[i] = displs[i] * recvext;
		}
		buffer = (char*)recvbuf;
	} else {
		// recv counts from parent (propagated modes)
		if (mpib_coll_sgv == 0 || mpib_coll_sgv == 2) {
			MPI_Recv(counts, size, MPI_INT, MPI_ANY_SOURCE, tag, comm, MPI_STATUS_IGNORE);
			builder.build(size, root, rank, counts, graph, r, u, v);
		}
		// prepare local buffer
		Tree tree = Tree(graph, r);
		int index;
		traverse_tree(v, tree,
			Indexer(counts, index, rank2index, _counts, _displs, count));
		buffer = (char*)malloc(sizeof(char) * count);
	}
	// copy own data
	MPI_Aint sendext;
	MPI_Type_extent(sendtype, &sendext);
	memcpy(buffer + _displs[rank2index[rank]], sendbuf, sendcount * sendext);
	// send counts to child (propagated modes)
	if (mpib_coll_sgv == 0 || mpib_coll_sgv == 2) {
		Adjacency_iterator ai, ai_end;
		tie(ai, ai_end) = adjacent_vertices(v, graph);
		while (ai != ai_end)
			MPI_Send(counts, size, MPI_INT, get(vertex_index, graph, (order == R2L) ? *(--ai_end) : *(ai++)), tag, comm);
	}
	// children's data
	Tree tree = Tree(graph, r);
	int subtree_size = rank2index.size() - 1;
	int c;
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
			Assembler(rank2index, _counts, _displs, c, lengths, indices));
		// recv data from child
		MPI_Type_indexed(c, lengths, indices, MPI_CHAR, &dts[child_counter]);
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
		MPI_Send(buffer, count, MPI_CHAR, get(vertex_index, graph, u), 0, comm);
	if (rank != root)
		free(buffer);
	free(_counts);
	free(_displs);
	free(counts);
	if (mpib_coll_sgv == 0)
		MPI_Comm_free(&comm);
	return MPI_SUCCESS;
}

#endif /*SGV_TREE_ALGORITHMS_HPP_*/
