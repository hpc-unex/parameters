#ifndef MPIB_COLL_BENCHMARKS_H_
#define MPIB_COLL_BENCHMARKS_H_

#include "mpib_measurement.h"

/*!
 * \defgroup coll_benchmarks Collective benchmarks
 * This module provides collective benchmarks based on root, max and global timing methods
 * (see \latexonly\cite{Lastovetsky2008MPIBlib}\endlatexonly).
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Container for a collective communication operation to be measured by \ref MPIB_measure_coll
 * (\ref MPIB_measure_max, \ref MPIB_measure_root, \ref MPIB_measure_global).
 * How to use (example in C):
 * - Create a data structure with the first field \ref MPIB_coll_container.
 * \code
typedef struct MPIB_Scatter_container {
    MPIB_coll_container base;
    char* buffer;
    MPIB_Scatter scatter;
} MPIB_Scatter_container;
 * \endcode
 * - Implement the functions: initialize, execute, finalize,
 * where \c _this argument can be typecasted to the data structure.
 * \code
void MPIB_Scatter_initialize(void* _this, MPI_Comm comm, int root, int M) {
    MPIB_Scatter_container* container = (MPIB_Scatter_container*)_this;
    int rank;
    MPI_Comm_rank(comm, &rank);
    int size;
    MPI_Comm_size(comm, &size);
    container->buffer = rank == root ?
        (char*)malloc(sizeof(char) * M * size) :
        (char*)malloc(sizeof(char) * M);
}

void MPIB_Scatter_execute(void* _this, MPI_Comm comm, int root, int M) {
    MPIB_Scatter_container* container = (MPIB_Scatter_container*)_this;
    container->scatter(container->base.buffer, M, MPI_CHAR,
        container->base.buffer, M, MPI_CHAR,
        root, comm);
}

void MPIB_Scatter_finalize(void* _this, MPI_Comm comm, int root) {
    MPIB_Scatter_container* container = (MPIB_Scatter_container*)_this;
    free(container->buffer);
}
 * \endcode
 * - Implement the functions that allocate and free the data structure.
 * \code
MPIB_Scatter_container* MPIB_Scatter_container_alloc(MPIB_Scatter scatter) {
    MPIB_Scatter_container* container =
        (MPIB_Scatter_container*)malloc(sizeof(MPIB_Scatter_container));
    container->base.operation = "Scatter";
    container->base.initialize = MPIB_Scatter_initialize;
    container->base.execute = MPIB_Scatter_execute;
    container->base.finalize = MPIB_Scatter_finalize;
    container->scatter = scatter;
    return container;
}

void MPIB_Scatter_container_free(void* _this) {
    free(_this);
}
 * \endcode+
 *
 * In this library, collective containers are implemented in C++.
 * \param comm MPI communicator over which the communication operation will be performed
 * \param root root process
 * \param M message size
 */
typedef struct MPIB_coll_container {
	/*! Communication operation */
	const char* operation;
	/*! Initialization of buffers required for the communication operation (in irregular collectives, M can be different at different processors) */
	int (*initialize)(void* _this, MPI_Comm comm, int root, int M);
	/*! Communication operation (in irregular collectives, M can be different at different processors) */
	int (*execute)(void* _this, MPI_Comm comm, int root, int M);
	/*! Finalization of buffers required for the communication operation */
	int (*finalize)(void* _this, MPI_Comm comm, int root);
} MPIB_coll_container;

/*!
 * Collective benchmark.
 * Measures the execution time of collective operation for a given message size.
 * \param container communication operaion container
 * \param comm communicator
 * \param root root process
 * \param M message size
 * \param precision measurement precision
 * \param result measurement result
 */
typedef int (*MPIB_measure_coll)(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result);

/*!
 * Collective benchmark for multiple message sizes.
 * Measures the execution time of collective operation for different message sizes.
 * \param container communication operaion container
 * \param comm communicator
 * \param root root process
 * \param msgset message sizes
 * \param precision measurement precision
 * \param count the number of measurements performed (significant only at the root processor)
 * \param results array of measurement results (significant only at the root processor,
 * allocated by this function and must be deallocated by user)
 */
typedef void (*MPIB_measure_coll_msgset)(MPIB_coll_container* container, MPI_Comm comm, int root,
		MPIB_msgset msgset,	MPIB_precision precision, int* count, MPIB_result** results);

/*!
 * Measures the execution time of collective operation at all processes and finds a maximum.
 * In the loop over repetitions:
 * - Synchronizes the processes by double barrier.
 * - Measures the execution time of collective operation at all processes.
 * - Finds maximum by allreduce.
 * - Performs statistical analysis.
 */
int MPIB_measure_max(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result);
int MPIB_measure_max_msgset(MPIB_coll_container* container, MPI_Comm comm, int root,
		MPIB_msgset msgset,	MPIB_precision precision, int* count, MPIB_result** results);

/*!
 * Measures the average execution time of barrier at all processes in the communicator
 * and sets up an internal global variable, which is used by \ref MPIB_measure_root.
 * Called by \ref MPIB_measure_root. Can be called directly before measurements.
 * \param comm communicator
 * \param reps number of repetitions (not precision - we suppose no pipeline effect with barrier)
 */
void MPIB_root_timer_init(MPI_Comm comm, int reps);

/*!
 * Measures the execution time of collective operation at the root process.
 * Reuses already obtained barrier time if the previous initialization was performed
 * over the same communicator. Otherwise, initializes the root timer by calling \ref MPIB_root_timer_init.\n
 * In the loop over repetitions:
 * - Synchronizes the processes by double barrier.
 * - Measures the execution time of collective operation and barrier confirmation at the root process.
 * - Subtracts the execution time of barrier.
 * - Performs statistical analysis at root.
 * 
 * Broadcasts the result.
 */
int MPIB_measure_root(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result);
int MPIB_measure_root_msgset(MPIB_coll_container* container, MPI_Comm comm, int root,
		MPIB_msgset msgset,	MPIB_precision precision, int* count, MPIB_result** results);

/*!
 * Measures the offsets between local clocks of of all processes in the communicator
 * and sets up an internal global variable, which is used by \ref MPIB_measure_global.
 * If MPI_WTIME_IS_GLOBAL (MPI global timer) is defined, the offsets are set to zero.
 * Otherwise, the offsets are measured.
 * Called by \ref MPIB_measure_global. Can be called directly before measurements.
 * \attention As the global variable is an array, the memory should be freed by
 * \code MPIB_global_timer_init(MPI_COMM_NULL, 0) \endcode
 * \param comm communicator
 * \param parallel several non-overlapped point-to-point communications at the same time if non-zero
 * \param reps number of repetitions (not precision because series of ping-pongs are required - we cannot
 * interrupt them by sending/receiving the result of the statistical estimation)
 */
void MPIB_global_timer_init(MPI_Comm comm, int parallel, int reps);

/*!
 * Measures the execution time of collective operation between processes using global time.
 * Based on \latexonly\cite{Worsch2002}\endlatexonly.
 * Reuses already obtained offsets between local clocks if the previous initialization was performed
 * on the same communicator. Otherwise, initializes the global timer by calling \ref MPIB_global_timer_init.\n
 * In the loop over repetitions:
 * - Synchronizes the processes by double barrier.
 * - Measures the moment of start at the root and the moment of finish at the rest of processes.
 * - Having substructed the offset, finds maximum by reducing to the root. 
 * - Performs statistical analysis at root.
 * 
 * Broadcasts the result.
 */
int MPIB_measure_global(MPIB_coll_container* container, MPI_Comm comm, int root,
		int M, MPIB_precision precision, MPIB_result* result);
int MPIB_measure_global_msgset(MPIB_coll_container* container, MPI_Comm comm, int root,
		MPIB_msgset msgset,	MPIB_precision precision, int* count, MPIB_result** results);


#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_COLL_BENCHMARKS_H_*/
