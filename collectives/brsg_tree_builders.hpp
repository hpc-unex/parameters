#ifndef TREE_BUILDERS_HPP_
#define TREE_BUILDERS_HPP_

#include "comm_tree.hpp"

namespace MPIB {

/*!
 * The communication tree builders for tree-based algorithms of bcast/reduce/scatter/gather must implement a function:
 * \code
void build(int size, int root, int rank, int count,
	Graph& g, Vertex& r, Vertex& u, Vertex& v);
 * \endcode
 * \param size communicator size
 * \param root root process
 * \param rank current process
 * \param count message size in bytes (count * extent)
 * \param g graph
 * \param r root vertex (introduced to avoid search)
 * \param u parent of v (introduced to avoid search)
 * \param v vertex for the current process (introduced to avoid search)
 */
namespace BRSG {
using namespace comm_tree;

/*!
 * Binomial tree builder for bcast/reduce/scatter/gather. The largest subtree on the right.
 * If root <> 0, processes' ranks will be cyclically shifted to 0.
 */
class Binomial_builder {
public:
	void build(int size, int root, int rank, int count,
			Graph& g, Vertex& r, Vertex& u, Vertex& v)
	{
		r = add_vertex(g);
		put(vertex_index, g, r, root);
		if (rank == root) {
			v = r;
		}
		// subtrees(parent vertex, root index, binomial size)
		deque<pair<Vertex, pair<int, int> > > subtrees;
		// n - 1 = sum of powers of 2
		for (int tmp = size - 1, bin = 1, index = 1; tmp > 0; tmp /= 2, bin *= 2) {
			if (tmp % 2) {
				subtrees.push_back(make_pair(r, make_pair(index, bin)));
				index += bin;
			}
		}
		// powers of 2 - DFS
		while (!subtrees.empty()) {
			pair<Vertex, pair<int, int> >& subtree = subtrees.front();
			Vertex s = subtree.first;
			Vertex t = add_vertex(g);
			// cyclic shift
			int index = subtree.second.first;
			int target = root + index;
			if (target >= size)
				target -= size;
			put(vertex_index, g, t, target);
			if (target == rank) {
				u = s;
				v = t;
			}
			pair<Edge, bool> e = add_edge(s, t, g);
			int bin = subtree.second.second;
			subtrees.pop_front();
			index += bin;
			for (bin /= 2; bin > 0; bin /= 2) {
				index -= bin;
				subtrees.push_front(make_pair(t, make_pair(index, bin)));
			}
		}
	}
};

}
}

#endif /*TREE_BUILDERS_HPP_*/
