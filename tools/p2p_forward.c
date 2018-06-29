/*!
 * \page p2p_forward Spawned process for scatter-gather point-to-point communication
 *	This program is launched by a master process performing scatter-gather based point-to-point
 *	communication. It can be launched a number of times on each host. The program listens for incoming
 *	control messages from any process in a loop:
 *	- if a control message has <code> info[0] != 1 </code>, then the program terminates
 *	- if a control message has <code> info[0] == 1 </code>, then the process receives a chunk of a message from a sender
 *	process and forwards the message to a receiver process
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "p2p/mpib_p2p.h"

#define MAX_ITER 16
#define MAX_STEPS 32

extern MPI_Comm intracomm, control_intracomm;

int MPIB_scatter_gather_based_p2p(void* sendbuf, int sendcount, int rest,
		MPI_Datatype sendtype, void* recvbuf, int recvcount,
		MPI_Datatype recvtype, int root, MPI_Comm comm, int source, int target,
		void* interbuf, int MPIB_p2p_blocking);

int main(int argc, char **argv) {

	char * s_array = NULL;
	char * r_array = NULL;
	int rank, size;
	int real_length;
	char name[128];
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	p2p_init(comm, 0);

	MPI_Comm_size(intracomm, &size);
	MPI_Comm_rank(intracomm, &rank);

	//SIZE = (SIZE/size) * size;
	MPI_Get_processor_name(name, &real_length);
	printf("I am a spawned process, rank %d of %d, name is %s\n", rank, size,
			name);

	int info[4];
	MPI_Status status;
	char *intermed_array;
	while (1) {
		//receive a message from whoever is the original sender
		MPI_Recv(&info, 4, MPI_INT, MPI_ANY_SOURCE, 0, control_intracomm,
				&status);
		//should I terminate or not?
		if (info[0] == 1) {
			int source = status.MPI_SOURCE; //this is the initial sender
			int target = info[1];
			int count = info[2];
			int rest = info[3];
			MPI_Datatype datatype = MPI_CHAR;
			intermed_array = (char *) malloc(count * sizeof(char));
			MPIB_scatter_gather_based_p2p(s_array, count, rest, datatype,
					r_array, count, datatype, -1, intracomm, source, target,
					intermed_array, 0);
			free(intermed_array);
		} else
			break;
	}

	p2p_finalize();
	MPI_Finalize();
	return 0;
}

