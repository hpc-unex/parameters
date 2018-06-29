#ifndef MPIB_COLL_H_
#define MPIB_COLL_H_

/*!
 * \page mpib_coll Collectives library
 * A shared library \c libmpib_coll.so implements different algorithms of collectives
 * - \ref basic_collectives (mostly linear)
 * - \ref tree_collectives (depends on the Boost C++ libraries)
 *
 * The command-line arguments:
 * - \b verbose verbose mode (default: no)
 * - \b sgv scatterv/gatherv mode
 *   - \e 0 propagated on duplicated communicator (default)
 *   - \e 1 broadcasted counts
 *   - \e 2 propagated with tags: not safe
 *   - \e 3 everywhere-defined counts: not standard
 *
 * \section tools Tools
 * - \subpage collective_test
 *
 * \section examples Examples
 * - \subpage substitute
 * - \subpage tree_builders
 * - \subpage sgv
 */

#include "mpib_basic_collectives.h"
#include "mpib_tree_collectives.h"

#endif /* MPIB_COLL_H_ */
