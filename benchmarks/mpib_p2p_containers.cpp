#include "mpib_p2p_benchmarks.h"
#include "p2p/mpib_p2p.h"
#include "mpib_defs.h"
#include <malloc.h>

class MPIB_roundtrip_container: public MPIB_p2p_container {
private:
	char* buffer;
public:
	MPIB_roundtrip_container() {
		MPIB_p2p_container::operation = "roundtrip";
		MPIB_p2p_container::initialize = initialize;
		MPIB_p2p_container::execute_measure_o_eager = execute_measure_o_eager;
		MPIB_p2p_container::execute_measure = execute_measure;
		MPIB_p2p_container::execute_measure_o_rdvz = execute_measure_o_rdvz;
		MPIB_p2p_container::execute_mirror_o = execute_mirror_o;
		MPIB_p2p_container::execute_mirror = execute_mirror;
		MPIB_p2p_container::execute_measure_Tm = execute_measure_Tm;
		MPIB_p2p_container::execute_mirror_Tm = execute_mirror_Tm;
		MPIB_p2p_container::finalize = finalize;
		
	}

	static void initialize(void* _this, MPI_Comm comm, int M) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		container->buffer = (char*)malloc(sizeof(char) * M);
	}

	static void execute_measure_o_eager(void* _this, MPI_Comm comm, int M, int mirror) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPI_Rsend(container->buffer, M, MPI_CHAR, mirror, 0, comm);
	
	}

	static void execute_mirror_o(void* _this, MPI_Comm comm, int M, int measure) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPI_Recv(container->buffer, M, MPI_CHAR, measure, 0, comm, MPI_STATUS_IGNORE);

	}
	
	static void execute_mirror(void* _this, MPI_Comm comm, int M, int measure) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPIB_Recv(container->buffer, M, MPI_CHAR, measure, 0, comm, MPI_STATUS_IGNORE);
		MPIB_Send(container->buffer, M, MPI_CHAR, measure, 0, comm);
	}
	
	static void execute_measure(void* _this, MPI_Comm comm, int M, int mirror) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPIB_Send(container->buffer, M, MPI_CHAR, mirror, 0, comm);
		MPIB_Recv(container->buffer, M, MPI_CHAR, mirror, 0, comm, MPI_STATUS_IGNORE);
	}	
	
	static void execute_measure_o_rdvz(void* _this, MPI_Comm comm, int M, int mirror) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPI_Ssend(container->buffer, M, MPI_CHAR, mirror, 0, comm);
	
	}

	static void execute_measure_Tm(void* _this, MPI_Comm comm, int M, int dest, int source) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPI_Sendrecv(container->buffer, M, MPI_CHAR, dest, 0, container->buffer, M, MPI_CHAR, source, 0, comm, MPI_STATUS_IGNORE);
		
	}

	static void execute_mirror_Tm(void* _this, MPI_Comm comm, int M, int dest, int source) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		MPI_Sendrecv(container->buffer, M, MPI_CHAR, dest, 0, container->buffer, M, MPI_CHAR, source, 0, comm, MPI_STATUS_IGNORE);
			
	}

	static void finalize(void* _this, MPI_Comm comm) {
		MPIB_roundtrip_container* container = (MPIB_roundtrip_container*)_this;
		free(container->buffer);
	}
};

extern "C" MPIB_p2p_container* MPIB_roundtrip_container_alloc() {
	return new MPIB_roundtrip_container();
}

extern "C" void MPIB_roundtrip_container_free(MPIB_p2p_container* container) {
	delete container;
}

