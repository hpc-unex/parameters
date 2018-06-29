#ifndef MPIB_OUTPUT_H_
#define MPIB_OUTPUT_H_

#include "mpib_measurement.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! Verbose mode (stderr) */
extern int MPIB_verbose;

/*! \brief Prints a list of processors (parallel) */
void MPIB_print_processors(MPI_Comm comm);

void MPIB_print_precision(MPIB_precision precision);
void MPIB_print_msgset(MPIB_msgset msgset);

void MPIB_print_result_th();
void MPIB_print_result_tr(MPIB_result result);

void MPIB_print_coll(const char* operation, const char* timing);
void MPIB_print_p2p(int parallel);

/*!
 * \brief Prints the per-p2p table of the p2p benchmark results.
 * \param M message size
 * \param parallel is parallel?
 * \param precision precision
 * \param n number of nodes
 * \param results array of \f$ C_n^2 \f$ results
 */
void MPIB_print_p2p_table(int M, int parallel, MPIB_precision precision, int n, MPIB_result* results);

/*!
 * \brief Prints the table header of the p2p benchmark results.
 * \param parallel is parallel?
 * \param precision precision
 * \param n number of nodes
 */
void MPIB_print_p2p_th(int parallel, MPIB_precision precision, int n);

/*!
 * \brief Prints the table row of the p2p benchmark results.
 * \param M message size
 * \param n number of nodes
 * \param results array of \f$ C_n^2 \f$ results
 */
void MPIB_print_p2p_tr(int M, int n, MPIB_result* results);

/*!
 * \brief Prints the table header of the collective benchmark result.
 * \details If operation == NULL, header is commented, with the time coulumn title "time".
 * Otherwise, header is uncommented (for gnuplot autoheader), with the time column title operation.
 * \param operation collective operation
 * \param timing timing method
 * \param n number of nodes
 * \param root root process
 * \param precision precision
 */
void MPIB_print_coll_th(const char* operation, const char* timing, int n, int root, MPIB_precision precision);

/*!
 * \brief Prints the table row of the collective benchmark result.
 * \param M message size
 * \param result measurement result
 */
void MPIB_print_coll_tr(int M, MPIB_result result);

#ifdef __cplusplus
}
#endif

#endif /*MPIB_OUTPUT_H_*/
