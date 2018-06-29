#include "libmpib_coll.h"
#include "sgv_flat.h"
#include <string.h>
#include <malloc.h>
#include <gsl/gsl_heapsort.h>

int MPIB_Scatterv_flat(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		MPI_Aint sendext;
		MPI_Type_extent(sendtype, &sendext);
		int size;
		MPI_Comm_size(comm, &size);
		int i;
		for (i = 0; i < size; i++) {
			if (i == root) {
				MPI_Aint recvext;
				MPI_Type_extent(recvtype, &recvext);
				memcpy(recvbuf, (char*)sendbuf + displs[i] * sendext, recvcount * recvext);
			} else
				MPI_Send((char*)sendbuf + displs[i] * sendext, sendcounts[i], sendtype, i, 0, comm);
		}
	} else
		MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
	return MPI_SUCCESS;
}

int MPIB_Gatherv_flat(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		MPI_Aint recvext;
		MPI_Type_extent(recvtype, &recvext);
		int size;
		MPI_Comm_size(comm, &size);
		int i;
		for (i = 0; i < size; i++) {
			if (i == root) {
				MPI_Aint sendext;
				MPI_Type_extent(sendtype, &sendext);
				memcpy((char*)recvbuf + displs[i] * recvext, sendbuf, sendcount * sendext);
			} else
				MPI_Recv((char*)recvbuf + displs[i] * recvext, recvcounts[i], recvtype, i, 0, comm, MPI_STATUS_IGNORE);
		}
	} else
		MPI_Send(sendbuf, sendcount, sendtype, root, 0, comm);
	return MPI_SUCCESS;
}

int MPIB_Scatterv_sorted_flat(MPIB_SGv_sorter* sorter,
	void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		int* counts = (int*)malloc(sizeof(int) * size);
		MPI_Aint sendext;
		MPI_Type_extent(sendtype, &sendext);
		int i;
		for (i = 0; i < size; i++)
			counts[i] = sendcounts[i] * sendext;
		size_t* indices = (size_t*)malloc(sizeof(size_t) * size);
		sorter->sort(sorter, size, counts, indices);
		for (i = 0; i < size; i++) {
			if (indices[i] == root) {
				MPI_Aint recvext;
				MPI_Type_extent(recvtype, &recvext);
				memcpy(recvbuf, (char*)sendbuf + displs[indices[i]] * sendext, recvcount * recvext);
			} else
				MPI_Send((char*)sendbuf + displs[indices[i]] * sendext, sendcounts[indices[i]], sendtype, indices[i], 0, comm);
		}
		free(counts);
		free(indices);
	} else
		MPI_Recv(recvbuf, recvcount, recvtype, root, 0, comm, MPI_STATUS_IGNORE);
	return MPI_SUCCESS;
}

int MPIB_Gatherv_sorted_flat(MPIB_SGv_sorter* sorter,
	void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	if (rank == root) {
		int size;
		MPI_Comm_size(comm, &size);
		int* counts = (int*)malloc(sizeof(int) * size);
		MPI_Aint recvext;
		MPI_Type_extent(recvtype, &recvext);
		int i;
		for (i = 0; i < size; i++)
			counts[i] = recvcounts[i] * recvext;
		size_t* indices = (size_t*)malloc(sizeof(size_t) * size);
		sorter->sort(sorter, size, counts, indices);
		for (i = 0; i < size; i++) {
			if (indices[i] == root) {
				MPI_Aint sendext;
				MPI_Type_extent(sendtype, &sendext);
				memcpy((char*)recvbuf + displs[indices[i]] * recvext, sendbuf, sendcount * sendext);
			} else
				MPI_Recv((char*)recvbuf + displs[indices[i]] * recvext, recvcounts[indices[i]], recvtype, indices[i], 0, comm, MPI_STATUS_IGNORE);
		}
		free(counts);
		free(indices);
	} else
		MPI_Send(sendbuf, sendcount, sendtype, root, 0, comm);
	return MPI_SUCCESS;
}

/*! Count sorter */
typedef struct MPIB_SGv_count_sorter {
	MPIB_SGv_sorter base;
	int order;
} MPIB_SGv_count_sorter;

/*!
 * Sorts counts. Result: indices (ranks).
 * We don't use the int gsl_sort_index because it provides only asc order.
 */
void MPIB_sort_counts(void* _this, const int size, const int* counts, size_t* indices) {
	int asc(const int* a, const int* b) {
		if (a > b)
			return 1;
		if (a < b)
			return -1;
		return 0;
	}
	int dsc(const int* a, const int* b) {
		if (a > b)
			return -1;
		if (a < b)
			return 1;
		return 0;
	}
	if (((MPIB_SGv_count_sorter*)_this)->order)
		gsl_heapsort_index(indices, counts, size, sizeof(int), (int (*)(const void*, const void*))dsc);
	else
		gsl_heapsort_index(indices, counts, size, sizeof(int), (int (*)(const void*, const void*))asc);
}

int MPIB_Scatterv_sorted_flat_asc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype, void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	MPIB_SGv_count_sorter sorter = {{MPIB_sort_counts}, 0};
	return MPIB_Scatterv_sorted_flat(&sorter.base, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);
}

int MPIB_Gatherv_sorted_flat_asc(void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	MPIB_SGv_count_sorter sorter = {{MPIB_sort_counts}, 0};
	return MPIB_Gatherv_sorted_flat(&sorter.base, sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm);
}

int MPIB_Scatterv_sorted_flat_dsc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype, void* recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	MPIB_SGv_count_sorter sorter = {{MPIB_sort_counts}, 1};
	return MPIB_Scatterv_sorted_flat(&sorter.base, sendbuf, sendcounts, displs, sendtype, recvbuf, recvcount, recvtype, root, comm);
}

int MPIB_Gatherv_sorted_flat_dsc(void* sendbuf, int sendcount, MPI_Datatype sendtype, void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype, int root, MPI_Comm comm) {
	MPIB_SGv_count_sorter sorter = {{MPIB_sort_counts}, 1};
	return MPIB_Gatherv_sorted_flat(&sorter.base, sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, root, comm);
}
