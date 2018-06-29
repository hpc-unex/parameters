#include "tree_algorithms.hpp"
#include "brsg_tree_builders.hpp"
#include "sgv_tree_builders.hpp"

using namespace MPIB;

extern "C" int MPIB_Bcast_binomial(void* buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm)
{
	return MPIB_Bcast_tree_algorithm(BRSG::Binomial_builder(), R2L,
		buffer, count, datatype, root, comm);
}

extern "C" int MPIB_Reduce_binomial(void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype,
	MPI_Op op, int root, MPI_Comm comm)
{
	return MPIB_Reduce_tree_algorithm(BRSG::Binomial_builder(), L2R,
		sendbuf, recvbuf, count, datatype,
		op, root, comm);
}

extern "C" int MPIB_Scatter_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Scatter_tree_algorithm(BRSG::Binomial_builder(), R2L,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Gather_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Gather_tree_algorithm(BRSG::Binomial_builder(), L2R,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Scatterv_binomial(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Scatterv_tree_algorithm(SGv::Binomial_builder(), R2L,
		sendbuf, sendcounts, displs, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Gatherv_binomial(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Gatherv_tree_algorithm(SGv::Binomial_builder(), L2R,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcounts, displs, recvtype,
		root, comm);
}

extern "C" int MPIB_Scatterv_sorted_binomial_asc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Scatterv_tree_algorithm(SGv::Sorted_binomial_builder(ASC), R2L,
		sendbuf, sendcounts, displs, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Gatherv_sorted_binomial_asc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Gatherv_tree_algorithm(SGv::Sorted_binomial_builder(ASC), L2R,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcounts, displs, recvtype,
		root, comm);
}

extern "C" int MPIB_Scatterv_sorted_binomial_dsc(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Scatterv_tree_algorithm(SGv::Sorted_binomial_builder(DESC), R2L,
		sendbuf, sendcounts, displs, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Gatherv_sorted_binomial_dsc(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Gatherv_tree_algorithm(SGv::Sorted_binomial_builder(DESC), L2R,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcounts, displs, recvtype,
		root, comm);
}

extern "C" int MPIB_Scatterv_Traff(void* sendbuf, int* sendcounts, int* displs, MPI_Datatype sendtype,
	void* recvbuf, int recvcount, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Scatterv_tree_algorithm(SGv::Traff_builder(), R2L,
		sendbuf, sendcounts, displs, sendtype,
		recvbuf, recvcount, recvtype,
		root, comm);
}

extern "C" int MPIB_Gatherv_Traff(void* sendbuf, int sendcount, MPI_Datatype sendtype,
	void* recvbuf, int* recvcounts, int* displs, MPI_Datatype recvtype,
	int root, MPI_Comm comm)
{
	return MPIB_Gatherv_tree_algorithm(SGv::Traff_builder(), L2R,
		sendbuf, sendcount, sendtype,
		recvbuf, recvcounts, displs, recvtype,
		root, comm);
}
