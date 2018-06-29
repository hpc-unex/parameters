#ifndef MPIB_MEASUREMENT_H_
#define MPIB_MEASUREMENT_H_

#include <mpi.h>

/*!
 * \defgroup measurement Measurement: base data structures and functions
 * This module provides base data structures and functions for measurement:
 * 
 * \dotfile benchmarks/benchmark.dot
 * 
 * Benchmark input:
 * - The \ref MPIB_precision data structure defines statistical precision of measurement.
 * Statistical analysis is implemented with help of the GNU Scientific Library.
 * - The \ref MPIB_p2p_container and \ref MPIB_coll_container data structures encapsulate
 * the communication operation to be measured.
 *
 * Benchmark output:
 * - The \ref MPIB_result data structure represents result of measurement and its reliability.
 *
 * Most benchmark functions have a counterpart for measurement of execution time
 * with a set of message sizes:
 *
 * \dotfile benchmarks/benchmark_msgset.dot
 *   
 * The counterpart has an extra input argument:
 * - The \ref MPIB_msgset data structure defines the message sizes for which the measurements
 * are to be performed. Message sizes are selected either regularly or adaptively.
 *
 * The output of the counterpart is an array of \ref MPIB_result.
 *
 * \{
 */ 
#ifdef __cplusplus
extern "C" {
#endif

/*! Creates a copy of communicator that includes one process per processor. */
void MPIB_Comm(MPI_Comm comm, MPI_Comm* newcomm);

/*! Result of measurement and its reliability. Used as an output argument of benchmark functions. */
typedef struct MPIB_result {
	/*! Message size */
	int M;
	/*! Execution time */
	double T;
	/*! Resolution of MPI_Wtime */
	double wtick;
	/*! Number of repetitions the benchmark has actually taken */
	int reps;
 	/*!
 	 * Confidence interval, \f$ |\bar T - \mu| < ci \f$.
 	 * The \ref MPIB_ci function estimates confidence interval, using t distribution.
 	 */
	double ci;
	;
}
MPIB_result;

/*!
 * The message sizes for which the measurements are to be performed.
 * Bounded by \ref min_size and \ref max_size.
 * If \ref stride > 0, message sizes are selected regularly.
 * Otherwise, they are adaptively selected at runtime,
 * based on the \ref max_diff, \ref min_stride and \ref max_num values.
 */
typedef struct MPIB_msgset {
	/*! Maximum message size in bytes */
	int min_size;
	/*! Maximum message size in bytes */
	int max_size;
	/*! Stride in bytes for regular selection of message sizes */
	int stride;
	/*!
	 * Maximum relative difference between the result of measurement and the linear model
	 * based on the results of two previous measurements that requires further investigation.
	 * Must be non-negative, \f$ \le 1 \f$. Used in adaptive selection of message sizes.
	 */
	double max_diff;
	/*!
	 * Minimum stride between message sizes. Must be positive.
	 * Used in adaptive selection of message sizes.
	 */
	int min_stride;
 	/*!
 	 * Msg size when eager protocol goes to rendevouz
 	 */
	int threshold;
	/*
	*  Type of communication for overheads and transfers times
	*/
	int loptype;
	/*!
 	 * Maximum number of message sizes. Limits the number of different messages sizes
 	 * the measurement is performed for. Used in adaptive selection of message sizes.
 	 */
	int max_num;
	/*
	* Communicator type ( 0 = TCP , 1 = IB)
	*/
	int comm_type;
}
MPIB_msgset;

/*!
 * Compares the result of measurement with the linear model based on the results of
 * two previous measurements. Required for adaptive selection of message sizes.
 */
double MPIB_diff(MPIB_result result, MPIB_result results[2]);

/*!
 * Returns a resolution of MPI_Wtime, maximum in the communicator.
 * Result can be used to check the execution time measured at several processors
 * (\ref MPIB_measure_max, \ref MPIB_measure_global): \f$ T_{coll} < wtick_{max} \f$.
 * \param comm MPI communicator
 * \param wtick a maximum resolution
 */
void MPIB_max_wtick(MPI_Comm comm, double* wtick);

/*!
 * Returns a confidence interval that contains the average execution time with a certain probability:
 * \f$ Pr(|\bar T - \mu| < ci) = cl \f$.
 * \note If communication operations in a series are isolated from each other,
 * we can assume that the execution times form an independent sample from a normally distributed population,
 * and use t distribution to estimate confidence interval.
 * \param cl confidence level
 * \param reps number of measurements (should be > 1)
 * \param T array of reps measurement results 
 * \return confidence interval
 */
double MPIB_ci(double cl, int reps, double* T);

/*!
 * Precision of measurement. Used as an input argument of benchmark functions. 
 * To provide reliable results, the communication experiments in each benchmark are repeated either fixed or
 * variable number of times. This data structure allows the user to control the accuracy and efficiency of benchmarking.
 * - Assigning to \ref min_reps and \ref max_reps the same values results in the fixed number of repetitions of
 * the communication operation, with the \ref cl and \ref eps arguments being ignored
 * (this allows the user to control the efficiency of benchmarking).
 * - If \ref min_reps < \ref max_reps, the experiments are repeated until a confidence interval, \ref MPIB_result::ci,
 * found with the confidence level, \ref cl = \f$ Pr(|\bar T - \mu| < ci) \f$, satisfies
 * \f$ \displaystyle\frac{ci}{\bar T} < \f$ \ref eps, or the number of repetitions reaches its maximum,
 * \ref max_reps (this allows the user to control the accuracy of benchmarking).
 */
typedef struct MPIB_precision {
	/*! Minimum number of repetitions */
	int min_reps;
	/*! Maximum number of repetitions */
	int max_reps;
	/*! Confidence level \f$ \in [0, 1] \f$: \f$ cl = Pr(|\bar T - \mu| < ci) = Pr(\displaystyle\frac{|\bar T - \mu|}{\bar T} < \epsilon) \f$. */
	double cl;
	/*! Relative error \f$ \in [0, 1] \f$: \f$ \displaystyle\frac{|\bar T - \mu|}{\bar T} < \frac{ci}{\bar T} < \epsilon = eps \f$. */
	double eps;
}
MPIB_precision;

#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_MEASUREMENT_H_*/
