/*!
 * \file
 * For internal use
 */
#ifndef MPIB_H_
#define MPIB_H_

/*!
 * \page mpib Benchmarks library
 * A static library \c libmpib.a implements point-to-point and collective benchmarks.
 * Main library modules (the modules marked grey can be extended, providing the benchmarking
 * of user-defined point-to-point and collective operations):
 * - \ref measurement (depends on the GNU Scientific Library)
 * - \ref benchmarks (now includes only benchmark for bcast)
 * - \ref p2p_benchmarks (seqential/parallel point-to-point benchmark)
 * - \ref p2p_containers (data structures describing point-to-point communications to be measured,
 * which are used as an argument of the point-to-point benchmark function)
 * - \ref coll_benchmarks (collective benchmarks based on root, max and global timing methods)
 * - \ref coll_containers (data structures describing collective communications to be measured,
 * which are used as an argument of collective benchmark functions)
 * - \ref defs (definitions of MPI communication operations)
 *
 * \dotfile benchmarks/modules.dot
 *
 * \section tools Tools
 * - \subpage p2p
 * - \subpage collective
 * - \subpage generate_factors
 * 
 * Command-line arguments are described in \ref getopt.
 *
 * \section tests Tests
 * - \subpage sg-based-p2p
 * - \subpage hybrid_fragment
 * - \subpage p2p_eager
 */

#include "mpib_coll_benchmarks.h"
#include "mpib_coll_containers.h"
#include "mpib_p2p_benchmarks.h"
#include "mpib_p2p_containers.h"
#include "mpib_getopt.h"
#include "mpib_shared.h"
#include "mpib_output.h"
#include "mpib_utilities.h"

#endif /*MPIB_H_*/
