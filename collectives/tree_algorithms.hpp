#ifndef TREE_ALGORITHMS_HPP_
#define TREE_ALGORITHMS_HPP_

/*!
 * \defgroup tree_algorithms Function templates for tree-based algorithms of MPI collective operations
 * In addition to the standard arguments of an MPI collective operation, a function template
 * has the communication tree builder and the order arguments:
 * \code
template <typename Builder>
MPIB_X_tree_algorithm(Builder builder, order args, standard args);
 * \endcode
 * For example, \ref MPIB_Scatter_tree_algorithm, a base tree algorithm of scatter.
 * In base tree-based algorithm, all point-to-point communications are performed over
 * the communication tree built by the builder in order given by the order argument.
 *
 * Usually, the communication tree is built at all processors independently.
 * If the communication tree can be built only at a designated processor,
 * it must then be sent to other processes along with the data.
 * The Serialization Boost C++ library is used for serialization/deserialization
 * of the communication tree/subtrees in such tree-based algorithms:
 * \code
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>

Graph graph;

if (rank == root) {
	ostringstream oss;
	archive::binary_oarchive ar(oss);
	ar << graph;
	int length = oss.str().length();
	MPI_Send((void*)oss.str().c_str(), length, MPI_CHAR, dest, 1, comm);
}

if (rank == dest) {
	MPI_Status status;
	MPI_Probe(root, 1, comm, &status);
	int length;
	MPI_Get_count(&status, MPI_CHAR, &length);
	buffer = (char*)malloc(sizeof(char) * length);
	MPI_Recv(buffer, length, MPI_CHAR, root, 1, comm, MPI_STATUS_IGNORE);
	istringstream iss(string(buffer, length));
	archive::binary_iarchive ar(iss);
	ar >> graph;
	free(buffer);
}
 * \endcode
 *
 * The internal part of the tree-based implementation includes the following auxiliaries
 * united in namespaces in order to avoid duplicates:
 * - <strong>Tree visitors</strong> traverse communication tree, for example, in order to
 * assemble the data buffer to send or receive:
 * \code
class Visitor {
public:
	Visitor(args) {...}
	void preorder(Vertex vertex, Tree& tree) {...}
	void inorder(Vertex vertex, Tree& tree) {...}
	void postorder(Vertex vertex, Tree& tree) {...}
};
 * \endcode
 * \note There may be many pointer or reference arguments in the visitor's constructor
 * because visitors are copied by value.
 * - <strong>Property writers</strong> print vertex, edge and graph properties during the output
 * of the communication tree:
 * \code
class Vertex_writer {
public:
	void operator()(std::ostream& out, const Vertex& v) const {
		out << "[label=\"" << ... << "\"]";
	}
};

class Edge_writer {
public:
	void operator()(std::ostream& out, const Edge& e) const {
		out << "[label=\"" << ... << "\"]";
	}
};

class Graph_writer {
public:
	void operator()(std::ostream& out) const {
		out << "graph [...]\n";
		out << "node [...]\n";
		out << "edge [...]\n";
	}
};

write_graphviz(cout, graph, Vertex_writer(), Edge_writer(), Graph_writer());
 * \endcode
 * \note Default writers are called when the last three arguments omitted.
 *
 * \{
 */

#include "br_tree_algorithms.hpp"
#include "sg_tree_algorithms.hpp"
#include "sgv_tree_algorithms.hpp"

/*!
 * \}
 */

#endif /* TREE_ALGORITHMS_HPP_ */
