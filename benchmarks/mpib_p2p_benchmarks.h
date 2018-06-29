#ifndef MPIB_P2P_BENCHMARK_H_
#define MPIB_P2P_BENCHMARK_H_

#include "mpib_measurement.h"

/*!
 * \defgroup p2p_benchmarks Point-to-point benchmarks
 * This module provides the point-to-point benchmarks.
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Container for a point-to-point communication operation to be measured by \ref MPIB_measure_allp2p.
 * How to use (example in C):
 * - Create a data structure with the first field \ref MPIB_p2p_container.
 * \code
typedef struct MPIB_roundtrip_container {
	MPIB_p2p_container base;
	char* buffer;
} MPIB_roundtrip_container;
 * \endcode
 * - Implement the functions: initialize, execute_measure, execute_mirror, finalize,
 * where \c _this argument can be typecasted to the data structure.
 * \code
void MPIB_Send_Recv_initialize(void* _this, MPI_Comm comm, int M) {
	MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
	container->buffer = (char*)malloc(sizeof(char) * M);
}

void MPIB_Send_Recv_execute_measure(void* _this, MPI_Comm comm, int M, int mirror) {
	MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
	MPI_Send(container->buffer, M, MPI_CHAR, mirror, 0, comm);
	MPI_Recv(container->buffer, M, MPI_CHAR, mirror, 0, comm, MPI_STATUS_IGNORE);
}

void MPIB_Send_Recv_execute_mirror(void* _this, MPI_Comm comm, int M, int measure) {
	MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
	MPI_Recv(container->buffer, M, MPI_CHAR, measure, 0, comm, MPI_STATUS_IGNORE);
	MPI_Send(container->buffer, M, MPI_CHAR, measure, 0, comm);
}

void MPIB_Send_Recv_finalize(void* _this, MPI_Comm comm) {
	MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
	free(container->buffer);
}
 * \endcode
 * - Implement the functions that allocate and free the data structure.
 * \code
MPIB_p2p_container* MPIB_roundtrip_container_alloc() {
	MPIB_p2p_container* container =
		(MPIB_p2p_container*)malloc(sizeof(MPIB_roundtrip_container));
	container->base.operation = "MPI_Send-MPI_Recv";
	container->base.free = MPIB_roundtrip_container_free;
	container->initialize = MPIB_Send_Recv_initialize;
	container->execute_measure = MPIB_Send_Recv_execute_measure;
	container->execute_mirror = MPIB_Send_Recv_execute_mirror;
	container->finalize = MPIB_Send_Recv_finalize;
	return container;
}

void MPIB_Send_Recv_container_free(void* _this) {
	free(_this);
}
 * \endcode
 *
 * \param comm MPI communicator over which the communication operation will be performed
 * \param M message size
 * \param mirror mirror process
 * \param measure measure process
 */
typedef struct MPIB_p2p_container {
	/*! Communication operation */
	const char* operation;
	/*! Initializion of buffers required for the communication operation. */
	void (*initialize)(void* _this, MPI_Comm comm, int M);
	/*! Part of communication at the measure side */
	void (*execute_measure_o_eager)(void* _this, MPI_Comm comm, int M, int mirror);
	void (*execute_measure_o_rdvz)(void* _this, MPI_Comm comm, int M, int mirror);
	void (*execute_measure_Tm)(void* _this, MPI_Comm comm, int M, int dest, int source);
	void (*execute_measure)(void* _this, MPI_Comm comm, int M, int mirror);
	void (*execute_mirror)(void* _this, MPI_Comm comm, int M, int mirror);
	/*! Part of communication at the mirror side */
	void (*execute_mirror_o)(void* _this, MPI_Comm comm, int M, int measure);
	void (*execute_mirror_Tm)(void* _this, MPI_Comm comm, int M, int dest, int source);
	/*! Finalization of buffers required for the communication operation */
	void (*finalize)(void* _this, MPI_Comm comm);
} MPIB_p2p_container;

/*!
 * Point-to-point benchmark. Estimates the execution time of the point-to-point communications
 * between a pair of processors in the MPI communicator.
 * Performs series of communication experiments to obtain reliable results.
 * \param container communication operation container
 * \param comm communicator, number of nodes should be \f$ \ge 2 \f$
 * \param measure measure processor
 * \param mirror mirror processor
 * \param M message size
 * \param precision measurement precision
 * \param result measurement result (significant only at the measure processor)
 */
void MPIB_measure_p2p(MPIB_p2p_container* container, MPI_Comm comm, int measure, int mirror,
		int M, MPIB_precision precision, MPIB_result* result);

/*!
 * Point-to-point benchmark. Estimates the execution time of the point-to-point communication
 * between a pair of processors in the MPI communicator for different message sizes.
 * Performs series of communication experiments to obtain reliable results.
 * \param container communication operation container
 * \param comm communicator, number of nodes should be \f$ \ge 2 \f$
 * \param measure measure processor
 * \param mirror mirror processor
 * \param msgset message sizes
 * \param precision measurement precision
 * \param count the number of measurements performed (significant only at the measure processor)
 * \param results array of measurement results (significant only at the measure processor,
 * allocated by this function and must be deallocated by user)
 */
void MPIB_measure_p2p_msgset(MPIB_p2p_container* container, MPI_Comm comm, int measure, int mirror,
		MPIB_msgset msgset, MPIB_precision precision, int* count, MPIB_result** results);

/*! \f$ C_n^2 \f$ */
#define MPIB_C2(n) (n) * ((n) - 1) / 2

/*!
 * For a symmetric square matrix stored in the array of \f$ C_n^2 \f$ elements,
 * returns the index of the \f$ (i, j) \f$ element, \f$ i \ne j < n \f$:
 * \f$ \displaystyle\frac{(n - 1) + (n - I)}{2} I + (J - I - 1) \f$, \f$ I = min(i, j) \f$, \f$ J = max(i, j) \f$
 */
#define MPIB_IJ2INDEX(n, i, j) (2 * (n) - ((i) < (j) ? (i) : (j)) - 1) * (((i) < (j) ? (i) : (j))) / 2 + (((i) < (j) ? (j) : (i))) - (((i) < (j) ? (i) : (j))) - 1

/*!
 * Point-to-point benchmark. Estimates the execution time of the point-to-point communications
 * between all pairs of processors in the MPI communicator.
 * Performs series of communication experiments to obtain reliable results.
 * \param container communication operation container
 * \param comm communicator, number of nodes should be \f$ \ge 2 \f$
 * \param parallel several non-overlapped point-to-point communications at the same time if non-zero
 * \param M message size
 * \param precision measurement precision
 * \param results array of \f$ C_n^2 \f$ measurement results
 */
void MPIB_measure_allp2p(MPIB_p2p_container* container, MPI_Comm comm, int parallel,
		int M, MPIB_precision precision, MPIB_result* results);


void MPIB_measure_overhead_eager(MPIB_p2p_container* container, MPI_Comm comm, int measure, int mirror,
		MPIB_msgset msgset, MPIB_precision precision, int* count, MPIB_result** results);

void MPIB_measure_overhead_rdvz(MPIB_p2p_container* container, MPI_Comm comm, int measure, int mirror,
		MPIB_msgset msgset, MPIB_precision precision, int* count, MPIB_result** results);

void MPIB_measure_transfer(MPIB_p2p_container* container, MPI_Comm comm, int measure, int mirror, MPIB_msgset msgset, MPIB_precision precision, int* count, MPIB_result** results, int num_procs);

#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_P2P_BENCHMARK_H_*/
