#include "mpib_coll_containers.hpp"

extern "C" void MPIB_coll_container_free(MPIB_coll_container* container) {
	delete container;
}

extern "C" MPIB_coll_container* MPIB_Scatter_container_alloc(MPIB_Scatter scatter) {
	return new MPIB_Scatter_container(scatter);
}

extern "C" MPIB_coll_container* MPIB_Gather_container_alloc(MPIB_Gather gather) {
	return new MPIB_Gather_container(gather);
}

extern "C" MPIB_coll_container* MPIB_Bcast_container_alloc(MPIB_Bcast bcast) {
	return new MPIB_Bcast_container(bcast);
}

extern "C" MPIB_coll_container* MPIB_Reduce_container_alloc(MPIB_Reduce reduce) {
	return new MPIB_Reduce_container(reduce);
}

extern "C" MPIB_coll_container* MPIB_Comm_dup_free_container_alloc() {
	return new MPIB_Comm_dup_free_container;
}

extern "C" MPIB_coll_container* MPIB_Scatterv_container_alloc(MPIB_Scatterv scatterv, const double* factors) {
	return new MPIB_Scatterv_container(scatterv, factors);
}

extern "C" MPIB_coll_container* MPIB_Gatherv_container_alloc(MPIB_Gatherv gatherv, const double* factors) {
	return new MPIB_Gatherv_container(gatherv, factors);
}
