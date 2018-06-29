#ifndef MPIB_TEST_H_
#define MPIB_TEST_H_

#include "benchmarks/mpib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Test for a scatter implementation.
 * \param scatter scatter implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_scatter(MPIB_Scatter scatter, MPI_Comm comm, int root, int* res);

/*!
 * Test for a gather implementation.
 * \param gather gather implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_gather(MPIB_Gather gather, MPI_Comm comm, int root, int* res);

/*!
 * Test for a scatterv implementation.
 * \param scatterv scatterv implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_scatterv(MPIB_Scatterv scatterv, MPI_Comm comm, int root, int* res);

/*!
 * Test for a gatherv implementation.
 * \param gatherv gatherv implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_gatherv(MPIB_Gatherv gatherv, MPI_Comm comm, int root, int* res);

/*!
 * Test for a bcast implementation.
 * \param bcast bcast implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_bcast(MPIB_Bcast bcast, MPI_Comm comm, int root, int* res);

/*!
 * Test for a reduce implementation.
 * \param reduce reduce implementation
 * \param comm communicator
 * \param root root process
 * \param res result (0 - ok)
 */
void MPIB_test_reduce(MPIB_Reduce reduce, MPI_Comm comm, int root, int* res);

#ifdef __cplusplus
}
#endif

/*!
 * \}
 */

#endif /*MPIB_TEST_H_*/
