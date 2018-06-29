#ifndef SGV_TREE_BUILDERS_HPP_
#define SGV_TREE_BUILDERS_HPP_

#include "mpib_coll.h"
#include "comm_tree.hpp"
#include <functional>
#include <algorithm>

namespace MPIB {

/*!
 * Communication tree builders for scatterv/gatherv. Must implement a function:
 * \code
void build(int size, int root, int rank, int* counts,
	Graph& g, Vertex& r, Vertex& u, Vertex& v);
 * \endcode
 * \param size communicator size
 * \param root root process
 * \param rank current process
 * \param counts array of message sizes in bytes (counts * extent) (number of elements = communicator size)
 * \param g graph
 * \param r root vertex (introduced to avoid search)
 * \param u parent of v (introduced to avoid search)
 * \param v vertex for the current process (introduced to avoid search)
 * 
 * \see \ref sgv
 */
namespace SGv {
using namespace comm_tree;

/*!
 * Binomial tree builder for scatterv/gatherv. The largest subtree on the right.
 * If root <> 0, processes' procs will be cyclically shifted to 0.
 */
class Binomial_builder {
public:
	void build(int size, int root, int rank, int* counts,
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

struct second_less: public binary_function<pair<int, int>, pair<int, int>, bool> {
	bool operator()(const pair<int, int>& x, const pair<int, int>& y) const {
		return x.second < y.second;
	}
};

struct second_greater: public binary_function<pair<int, int>, pair<int, int>, bool> {
	bool operator()(const pair<int, int>& x, const pair<int, int>& y) const {
		return x.second > y.second;
	}
};

/*!
 * Sorted binomial tree builder for scatterv/gatherv. The largest subtree on the right.
 * Processors are sorted by counts in asc/dsc order.
 * Algorithm is described in \latexonly\cite{Traff2004}\endlatexonly.
 */
class Sorted_binomial_builder {
private:
	MPIB_sort_order selectedOrder;
public:
	Sorted_binomial_builder(MPIB_sort_order _selectedOrder): selectedOrder(_selectedOrder) {}

	void build(int size, int root, int rank, int* counts,
		Graph& g, Vertex& r, Vertex& u, Vertex& v)
	{
		// procs(rank, counts[rank]) except for root sorted by counts in descending order
		vector<pair<int, int> > procs;
		for (int i = 0; i < size; i++)
			if (i != root)
				procs.push_back(make_pair(i, counts[i]));
		if (selectedOrder == ASC)
			sort(procs.begin(), procs.end(), second_less());
		else
			sort(procs.begin(), procs.end(), second_greater());
		r = add_vertex(g);
		put(vertex_index, g, r, root);
		if (rank == root) {
			v = r ;
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
			int index = subtree.second.first;
			int target = procs[index - 1].first;
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

/*!
 * Traff tree builder for scatterv/gatherv.
 * Algorithm is described in \latexonly\cite{Traff2004}\endlatexonly.
 */
class Traff_builder {
public:
	void build(int size, int root, int rank, int* counts,
		Graph& g, Vertex& r, Vertex& u, Vertex& v)
	{
		// procs(rank, counts[rank]) except for root sorted by counts in descending order
		deque<pair<int, int> > procs;
		for (int i = 0; i < size; i++)
			if (i != root)
				procs.push_back(make_pair(i, counts[i]));
		sort(procs.begin(), procs.end(), second_greater());
		r = add_vertex(g);
		put(vertex_index, g, r, root);
		if (rank == root)
			v = r;
		// sets(source vertex, procs)
		deque<pair<Vertex, deque<int> > > sets;
		int sum = 0;
		int weight = 0;
		for (deque<pair<int, int> >::iterator i = procs.begin(); i != procs.end(); i++) {
			if (weight == 0)
				sets.push_back(make_pair(r, deque<int>()));
			sets.back().second.push_back(i->first);
			weight += i->second;
			if (weight >= sum) {// "weight > sum" is it a mistake in the Traff paper?
				sum += weight;
				weight = 0;
			}
		}
		// subtrees
		while (!sets.empty()) {
			// parent is the first
			Vertex s = sets.front().first;
			deque<int>& procs = sets.front().second;
			int target = procs.front();
			procs.pop_front();
			Vertex t = add_vertex(g);
			put(vertex_index, g, t, target);
			add_edge(s, t, g);
			if (rank == target) {
				u = s;
				v = t;
			}
			// children
			int sum = 0;
			int weight = 0;
			for (deque<int>::iterator i = procs.begin(); i != procs.end(); i++) {
				if (weight == 0)
					sets.push_back(make_pair(t, deque<int>()));
				int rank = *i;
				sets.back().second.push_back(rank);
				weight += counts[rank];
				if (weight >= sum) {
					sum += weight;
					weight = 0;
				}
			}
			sets.pop_front();
		}
	}
};

}
}

#endif /*SGV_TREE_BUILDERS_HPP_*/
