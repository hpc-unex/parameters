/*!
 * \page tree_builders Test for tree builders
 * Usage:
 * - include a header file with your builder class
 * - use your builder class in the main function
 *
 * \include tree_builders.cpp
 */
#include <getopt.h>

#include <boost/graph/graphviz.hpp>

#include "collectives/brsg_tree_builders.hpp"
using namespace MPIB::BRSG;

int main(int argc, char** argv) {
	int n = 8;
	int c;
	while ((c = getopt(argc, argv, "hn:")) != -1) {
		switch (c) {
			case 'h':
				cerr <<
"usage: trees [options]\n\
-h			help\n\
-n I			number of nodes in the tree\n";
				return 0;
				break;
			case 'n':
				n = atoi(optarg);
				break;
		}
	}
	Graph graph;
	Vertex r, u, v;
	Binomial_builder builder;
	builder.build(n, 0, 0, 0, graph, r, u, v);
	cout << "#Tree\n";
	write_graphviz(cout, graph);
	return 0;
}
