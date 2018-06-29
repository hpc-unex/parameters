#ifndef SGV_FLAT_H_
#define SGV_FLAT_H_

#include <mpi.h>
#include <stddef.h>

/*! Sorter for base sorted flat-tree scatterv/gatherv */
typedef struct MPIB_SGv_sorter {
	/*!
	 * \param _this self pointer
	 * \param size number of processes
	 * \param counts message sizes (in bytes)
	 * \param indices sorted ranks (result), must be pre-allocated
	 */
	void (*sort)(void* _this, const int size, const int* counts, size_t* indices);
} MPIB_SGv_sorter;

/*! Base sorted flat-tree scatterv */
int MPIB_Scatterv_sorted_flat(MPIB_SGv_sorter* sorter,
	void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

/*! Base sorted flat-tree gatherv */
int MPIB_Gatherv_sorted_flat(MPIB_SGv_sorter* sorter,
	void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm);

#endif /* SGV_FLAT_H_ */
