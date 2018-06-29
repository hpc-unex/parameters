#include "mpib_test.h"

#include <malloc.h>
#include <string.h>

#include "benchmarks/mpib_coll_containers.h"

void MPIB_test_scatter(MPIB_Scatter scatter, MPI_Comm comm, int root, int* res)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	int size;
	MPI_Comm_size(comm, &size);
	int M = 1024;
	int* sendbuf = NULL;
	if (rank == root)
	{
		sendbuf = (int*)malloc(sizeof(int) * M);
		int i;
		for (i = 0; i < M; i++)
			sendbuf[i] = i;
	}
	int m = M / size;
	int* recvbuf = (int*)calloc(m, sizeof(int));
	scatter(sendbuf, m, MPI_INT, recvbuf, m, MPI_INT, root, comm);
	int flag = 0;
	int i;
	for (i = 0; i < m; i++)
		flag |= recvbuf[i] != i + rank * m;
	MPI_Allreduce(&flag, res, 1, MPI_INT, MPI_MAX, comm);
	free(sendbuf);
	free(recvbuf);
}

void MPIB_test_gather(MPIB_Gather gather, MPI_Comm comm, int root, int* res)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	int size;
	MPI_Comm_size(comm, &size);
	int M = 1024;
	int* recvbuf = NULL;
	if (rank == root)
		recvbuf = (int*)calloc(M, sizeof(int));
	int m = M / size;
	int* sendbuf = (int*)malloc(sizeof(int) * m);
	int i;
	for (i = 0; i < m; i++)
		sendbuf[i] = i + rank * m;
	gather(sendbuf, m, MPI_INT, recvbuf, m, MPI_INT, root, comm);
	if (rank == root)
	{
		*res = 0;
		int i, s;
		for (i = 0, s = m * size; i < s; i++)
			*res |= recvbuf[i] != i;
	}
	MPI_Bcast(res, 1, MPI_INT, root, comm);
	free(sendbuf);
	free(recvbuf);
}

void MPIB_test_scatterv(MPIB_Scatterv scatterv, MPI_Comm comm, int root, int* res) {
	int rank;
	MPI_Comm_rank(comm, &rank);
	int size;
	MPI_Comm_size(comm, &size);
	int M = 1024;
	int m = M / size;
	int d = m / size;
	int* sendbuf = (int*)malloc(sizeof(int) * M);
	int i;
	for (i = 0; i < M; i++)
		sendbuf[i] = i;
	int* sendcounts = (int*)malloc(sizeof(int) * size);
	int* displs = (int*)malloc(sizeof(int) * size);
	for (i = 0; i < size; i++) {
		sendcounts[i] = m + (i % 2 ? 1 : -1) * d;
		displs[i] = i == 0 ? 0 : displs[i - 1] + sendcounts[i - 1];
	}
	int recvcount = m + (rank % 2 ? 1 : -1) * d;
	int* recvbuf = (int*)calloc(recvcount, sizeof(int));
	scatterv(sendbuf, sendcounts, displs, MPI_INT, recvbuf, recvcount, MPI_INT, root, comm);
	free(sendcounts);
	free(displs);
	int flag = 0;
	for (i = 0; i < recvcount; i++)
		flag |= recvbuf[i] != i + rank * m - (rank % 2 ? d : 0);
	MPI_Allreduce(&flag, res, 1, MPI_INT, MPI_MAX, comm);
	free(sendbuf);
	free(recvbuf);
}

void MPIB_test_gatherv(MPIB_Gatherv gatherv, MPI_Comm comm, int root, int* res) {
	int rank;
	MPI_Comm_rank(comm, &rank);
	int size;
	MPI_Comm_size(comm, &size);
	int M = 1024;
	int m = M / size;
	int d = m / size;
	int* recvbuf = (int*)calloc(M, sizeof(int));
	int* recvcounts = (int*)malloc(sizeof(int) * size);
	int* displs = (int*)malloc(sizeof(int) * size);
	int i;
	for (i = 0; i < size; i++) {
		recvcounts[i] = m + (i % 2 ? 1 : -1) * d;
		displs[i] = i == 0 ? 0 : displs[i - 1] + recvcounts[i - 1];
	}
	int sendcount = m + (rank % 2 ? 1 : -1) * d;
	int* sendbuf = (int*)malloc(sizeof(int) * sendcount);
	for (i = 0; i < sendcount; i++)
		sendbuf[i] = i + rank * m - (rank % 2 ? d : 0);
	gatherv(sendbuf, sendcount, MPI_INT, recvbuf, recvcounts, displs, MPI_INT, root, comm);
	free(recvcounts);
	free(displs);
	if (rank == root) {
		*res = 0;
		int i, s;
		for (i = 0, s = m * size - (size % 2 ? d : 0); i < s; i++)
			*res |= recvbuf[i] != i;
	}
	MPI_Bcast(res, 1, MPI_INT, root, comm);
	free(sendbuf);
	free(recvbuf);
}

void MPIB_test_bcast(MPIB_Bcast bcast, MPI_Comm comm, int root, int* res)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	int M = 1024;
	int* buffer = (int*)malloc(sizeof(int) * M);
	if (rank == root)
	{
		int i;
		for (i = 0; i < M; i++)
			buffer[i] = i;
	}
	bcast(buffer, M, MPI_INT, root, comm);
	int flag = 0;
	int i;
	for (i = 0; i < M; i++)
		flag |= buffer[i] != i;
	free(buffer);
	MPI_Allreduce(&flag, res, 1, MPI_INT, MPI_MAX, comm);
}

void MPIB_test_reduce(MPIB_Reduce reduce, MPI_Comm comm, int root, int* res)
{
	int rank;
	MPI_Comm_rank(comm, &rank);
	int M = 1024;
	int* recvbuf = rank == root ? (int*)malloc(sizeof(int) * M) : NULL;
	int* sendbuf = (int*)malloc(sizeof(int) * M);
	int i;
	for (i = 0; i < M; i++)
	{
		if (rank > 1)
			sendbuf[i] = i % rank ? 0 : i;
		else
			sendbuf[i] = 0;
	}
	reduce(sendbuf, recvbuf, M, MPI_INT, MPI_MAX, root, comm);
	free(sendbuf);
	int flag = 0;
	if (rank == root)
	{
		int size;
		MPI_Comm_size(comm, &size);
		int i;
		for (i = 0; i < M; i++)
		{
			int r;
			for (r = 2; r < size; r++)
			{
				if (i % r == 0)
				{
					flag |= recvbuf[i] != i;
					break;
				}
			}
			if (r == size)
				flag |= recvbuf[i] != 0;
		}
	}
	free(recvbuf);
	MPI_Bcast(&flag, 1, MPI_INT, root, comm);
	*res = flag;
}
