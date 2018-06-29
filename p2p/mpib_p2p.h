#ifndef MPIB_P2P_H_
#define MPIB_P2P_H_ 1

#include <mpi.h>

/*!
 * \page libmpib_p2p P2P library
 * A static library \c libmpib_p2p.a implements different algorithms of p2p communications.
 * 	- The API for using the modified p2p communication is available here: \ref mpib_p2p.
 * 	- The spawned process which is always used for the modified p2p communication is \subpage p2p_forward
 *
 * \defgroup mpib_p2p Wrappers for p2p operations
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif 

/*!
 * This init routine is called by all processes in the original MPI
 * communicator. Process 0 spawns a number of additional MPI processes everywhere (see \ref p2p_forward)
 * for the scatter-gather-based point-to-point communication. After the routine, the global communicator
 * intracomm contains all processes (spawning and spawned)
 */
int p2p_init(MPI_Comm comm, int proc_num_spawned);

/*!
 * This finalize routine is used by process 0 to send a terminate signal
 * to all spawned processes
 *  */
int p2p_finalize();

/*! Wrapper send function for standard or modified operation */
int MPIB_Send(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

/*! Wrapper non-blocking send function for standard or modified operation */
int MPIB_Isend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, MPI_Request* request);

/*! Wrapper receive function for standard or modified operation */
int MPIB_Recv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status* status);

/*! Wrapper non-blocking receive function for standard or modified operation */
int MPIB_Irecv(void* buf, int count, MPI_Datatype datatype, int source,
        int tag, MPI_Comm comm, MPI_Request* request);

/*! Wrapper waitall function for standard or modified operation */
int MPIB_Waitall(int count, MPI_Request* array_of_requests, MPI_Status* array_of_statuses);

#ifdef __cplusplus
}
#endif 
/*!
 * \}
 */

#endif /* MPIB_P2P_H_ */
