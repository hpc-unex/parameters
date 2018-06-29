#ifndef LIBMPIB_COLL_H_
#define LIBMPIB_COLL_H_

/*! Sorting order */
typedef enum {ASC, DESC} MPIB_sort_order;

/*! Traversing order */
typedef enum {L2R, R2L} MPIB_child_traverse_order;

/*! Verbose mode (stderr) */
extern int mpib_coll_verbose;

/*!
 * scatterv/gatherv mode:
 * - \i 0 propagated on duplicated communicator (default)
 * - \i 1 broadcasted counts
 * - \i 2 propagated with tags: not safe
 * - \i 3 everywhere-defined counts: not standard
 */
extern int mpib_coll_sgv;

#endif /* LIBMPIB_COLL_H_ */
