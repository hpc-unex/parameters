#ifndef MPIB_UTILITIES_H_
#define MPIB_UTILITIES_H_

/*! List of pairs */
typedef struct MPIB_pair
{
	/*! Values */
	int values[2];
	/*! Previous pair */
	struct MPIB_pair* prev;
	/*! Next pair */
	struct MPIB_pair* next;
}
MPIB_pair;

/*! List of lists of pairs */
typedef struct MPIB_pairs
{
	/*! Items */
	MPIB_pair* list;
	/*! Previous list */
	struct MPIB_pairs* prev;
	/*! Next list */
	struct MPIB_pairs* next;
}
MPIB_pairs;

/*!
 * Builds all combinations of non-overlapped pairs of communicating nodes.
 * TODO: rewrite in C++ queue<queue<pair<int, int> > > + some access function from C
 */
MPIB_pairs* MPIB_build_pairs(int n);

/*! Frees the pairs recursively. */
void MPIB_free_pairs(MPIB_pairs* pairs);

#endif /*MPIB_UTILITIES_H_*/
