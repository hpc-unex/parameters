#ifndef MPIB_P2P_CONTAINERS_H_
#define MPIB_P2P_CONTAINERS_H_

#include "mpib_p2p_benchmarks.h"
#include "mpib_defs.h"

/*!
 * \defgroup p2p_containers Containers for point-to-point communication operations
 * Data structures describing point-to-point communications to be measured,
 * which are used as an argument of the point-to-point benchmark function.
 * \{
 */ 
#ifdef __cplusplus
extern "C" {
#endif

/*! Allocates a roundtrip container */
MPIB_p2p_container* MPIB_roundtrip_container_alloc();

/*! Frees the roundtrip container */
void MPIB_roundtrip_container_free(MPIB_p2p_container* container);

#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_P2P_CONTAINERS_H_*/
