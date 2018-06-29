#ifndef MPIB_BENCHMARKS_H_
#define MPIB_BENCHMARKS_H_

#include "mpib_measurement.h"
#include "mpib_defs.h"

/*!
 * \defgroup benchmarks Operation-specific benchmarks
 * This module provides operation-specific benchmarks.
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Performs the p2p benchmark with empty message with given precision
 * and sets up an internal global variable, which is used by \ref MPIB_measure_bcast.
 * Called by \ref MPIB_measure_bcast. Can be called directly before measurements.
 * \attention As the global variable is an array, the memory should be freed by
 * \code MPIB_bcast_timer_init(MPI_COMM_NULL, (MPIB_precision){0, 0, 0}) \endcode
 * \param comm communicator
 * \param parallel several non-overlapped point-to-point communications at the same time if non-zero
 * \param precision measurement precision
 */
void MPIB_bcast_timer_init(MPI_Comm comm, int parallel, MPIB_precision precision);

/*!
 * Measures the execution time of bcast between root and the rest of processes.
 * Based on \latexonly\cite{Supinski1999}\endlatexonly.
 * Reuses already obtained empty-roundtrip times if the previous initialization was performed
 * on the same communicator. Otherwise, initializes the bcast timer by calling \ref MPIB_bcast_timer_init.\n
 * In the loop over processes, except root:\n
 * In the loop over repetitions:
 * - bcast at all processes except root and current process in the loop
 * - bcast and empty recv at root and measures the execution time
 * - bcast and empty send at current process in the loop
 * 
 * Having substructed the half of empty-roundtrip times, finds maximum.\n 
 * Broadcasts the result.
 * \note No double barriers as no need in synchronization.
 * \note No precision as we cannot interrupt the process of measurement by broadcasting the error value
 * after each repetition.
 * \param bcast bcast implementation
 * \param comm communicator
 * \param root root process
 * \param M message size
 * \param max_reps maximum number of repetitions
 * \param result measurement result
 */
void MPIB_measure_bcast(MPIB_Bcast bcast, MPI_Comm comm, int root, int M,
	int max_reps, MPIB_result* result);

#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_BENCHMARKS_H_*/
