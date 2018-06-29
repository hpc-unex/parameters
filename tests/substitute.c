/*!
 * \page substitute Substitute for native MPI collectives
 * Substitute for native MPI collective operations in parallel applications
 * - add <code>-I$(MPIBlib-dir)/include</code> to CPPFLAGS
 * - add <code>-L$(MPIBlib-dir)/lib -lmpib_coll</code> to LDFLAGS
 * - replace <code>\#include <mpi.h></code> by <code>\#include "mpib_coll.h"</code>
 * - define macro substitutes for native MPI collective operations <code>\#define MPI_Scatter MPIB_Scatter_flat</code>
 *
 * \include substitute.c
 */
//#include <mpi.h>
#include "mpib_coll.h"
#define MPI_Scatter MPIB_Scatter_flat_nb
#define MPI_Gather MPIB_Gather_flat_nb

#include <malloc.h>

int main(int argc, char** argv)
{
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	int M = 1024;
	char* buffer = rank == 0 ? (char*)malloc(sizeof(char) * M * size) : (char*)malloc(sizeof(char) * M);
	MPI_Scatter(buffer, M, MPI_CHAR, buffer, M, MPI_CHAR, 0, MPI_COMM_WORLD);
	MPI_Gather(buffer, M, MPI_CHAR, buffer, M, MPI_CHAR, 0, MPI_COMM_WORLD);
	free(buffer);
	MPI_Finalize();
	return 0;
}
