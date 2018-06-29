#ifndef MPIB_GETOPT_H_
#define MPIB_GETOPT_H_

#include "mpib_measurement.h"
#include "mpib_coll_benchmarks.h"

/*!
 * \defgroup getopt Options for executables
 * Since getopt cannot be reused, this module provides an interface for getopt, which can be used as follows:
 * \code
	X x;
	Y y;
	MPIB_getopt_x_default(&x);
	MPIB_getopt_y_default(&y);
	if (root == 0)
	{
		char options[256] = "";
		strcat(options, MPIB_getopt_x_options());
		strcat(options, MPIB_getopt_y_options());
		while ((c = getopt(argc, argv, options)) != -1)
		{
			MPIB_getopt_x_optarg(c, &x);
			MPIB_getopt_y_optarg(c, &y);
		}
	}
	MPIB_getopt_x_bcast(&x, 0, MPI_COMM_WORLD);
	MPIB_getopt_y_bcast(&y, 0, MPI_COMM_WORLD);
 * \endcode
 * 
 * Options are divided into sets, which are managed by the following functions:
 * - \b MPIB_getopt_X_default - fills the parameters by default values
 * - \b MPIB_getopt_X_options - returns a string of the getopt options
 * - \b MPIB_getopt_X_optarg - fills the parameters by the getopt argument
 * - \b MPIB_getopt_X_bcast - broadcasts the parameters (parallel)
 * where X stands for a name of the set.
 * 
 * Typical options for executables are as follows:
 * - \b -h help
 * - \b -v verbose
 * - \b -l \e S collectives shared library (required: a full path or relative to LD_LIBRARY_PATH)
 * - \b -o \e S suboptions (comma separated options for the shared library: \c subopt1,subopt2=value2)
 * - \b -O \e S collective operation defined in the shared library (required: the name must contain Bcast, Scatter, etc)
 * - \b -t \e S timing: max, root, global (default: max)
 * - \b -m \e I minimum message size (default: 0)
 * - \b -M \e I maximum message size (default: 204800)
 * - \b -S \e I stride between message sizes (default: 1024)
 * - \b -d \e D maximum relative difference: 0 < D < 1 (default: 0.1)
 * - \b -s \e I minimum stride between message sizes (default: 64)
 * - \b -n \e I maximum number of message sizes (default: 100)
 * - \b -p 0/1 parallel p2p benchmarking (default: 1)
 * - \b -r \e I minimum number of repetitions (default: 5)
 * - \b -R \e I maximum number of repetitions (default: 100)
 * - \b -c \e D confidence level: 0 < D < 1 (default: 0.95)
 * - \b -e \e D relative error: 0 < D < 1 (default: 0.025)
 * - \b -p \e I parallel mode or not: 0 / 1 (default: 0)
 * - \b -P \e I type of P2P experiments: 0 is default, 1 is scatter/gather based P2P: 0 / 1 (default: 0). Note: If using -P 1 you have to be in the <install-dir>/bin directory
 * 
 * where:
 * - \e S - string
 * - \e I - integer
 * - \e D - double
 * 
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif
void MPIB_getopt_help_default(int* exit);
void MPIB_getopt_multicore_default(int* multicore);
const char* MPIB_getopt_help_options();
void MPIB_getopt_help_optarg(int c, const char* program, int* exit);
void MPIB_getopt_help_bcast(int* exit, int root, MPI_Comm comm);

void MPIB_getopt_msgset_default(MPIB_msgset* msgset);
const char* MPIB_getopt_msgset_options();
void MPIB_getopt_msgset_optarg(int c, MPIB_msgset* msgset);
void MPIB_getopt_msgset_bcast(MPIB_msgset* msgset, int root, MPI_Comm comm);

void MPIB_getopt_precision_default(MPIB_precision* precision);
const char* MPIB_getopt_precision_options();
void MPIB_getopt_precision_optarg(int c, MPIB_precision* precision);
void MPIB_getopt_precision_bcast(MPIB_precision* precision, int root, MPI_Comm comm);

void MPIB_getopt_p2p_default(int* parallel, int* p2p_type, int* proc_num_spawned);
const char* MPIB_getopt_p2p_options();
void MPIB_getopt_p2p_optarg(int c, int* parallel, int* p2p_type, int* proc_num_spawned);
void MPIB_getopt_p2p_bcast(int* parallel, int *p2p_type, int* proc_num_spawned, int root, MPI_Comm comm);

void MPIB_getopt_coll_default(char* timing);
const char* MPIB_getopt_coll_options();
void MPIB_getopt_coll_optarg(int c, char* timing);
void MPIB_getopt_coll_bcast(char* timing, MPIB_measure_coll* measure, int root, MPI_Comm comm);
#ifdef __cplusplus
}
#endif
/*!
 * \}
 */

#endif /*MPIB_GETOPT_H_*/
