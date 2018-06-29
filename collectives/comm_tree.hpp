#ifndef COMM_TREE_HPP_
#define COMM_TREE_HPP_

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_as_tree.hpp>

namespace MPIB {
/*!
 * <strong>Communication tree</strong> is a C++ namespace that contains a set of data structures
 * for tree-based algorithms of collectives. All data types are short aliases for the Boost Graph data structures.
 */
namespace comm_tree {
using namespace boost;
using namespace std;

/*!
 * Directed graph (vertex_index = MPI rank)
 * \note The vertex index property is used for indexing and must starts from zero.
 * \note The adjacent_vertices method provides only the parent->child relation.
 * For backward connection, use the graph as tree wrapper \ref Tree.
 */
typedef adjacency_list<listS, listS, directedS, property<vertex_index_t, int>, no_property, no_property, listS> Graph;

/*! Vertex */
typedef graph_traits<Graph>::vertex_descriptor Vertex;

/*! Edge */
typedef graph_traits<Graph>::edge_descriptor Edge;

/*! Vertex iterator */
typedef graph_traits<Graph>::vertex_iterator Vertex_iterator;

/*! Adjacency iterator */
typedef graph_traits<Graph>::adjacency_iterator Adjacency_iterator;

/*!
 * Graph as tree wrapper. Provides access to the root, parent and children nodes.
 * \note We cannot use the \c Tree data structure directly, instead of \c Graph, because
 * of the lack of empty/copy constructors.
 * \note Access to parent nodes requires some computation (traversing the tree) - avoid this if possible.
 */
typedef graph_as_tree<Graph,
	iterator_property_map<vector<Vertex>::iterator,	property_map<Graph, vertex_index_t>::type> > Tree;

}
}

#endif /*COMM_TREE_HPP_*/
