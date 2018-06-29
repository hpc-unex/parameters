/*!
 * \page p2p P2P benchmark
 * Performs adaptive p2p benchmark between a pair of processors 0-1.
 * Arguments are described in \ref getopt.
 * 
 * \b Example:
 * \code
 * $ mpirun -n 2 p2p -m 1024 -M 2049 > p2p.out
 * \endcode
 * will benchmark the p2p communication between 2 processes for
 * message sizes of 1024 and 2048 bytes
 * 
 * \b p2p.plot draws the graph of the execution time (sec) against message size (kb).
 * - input: p2p.out
 * - output: p2p.eps (0-1 with error bars)
 *
 * Using the gnuplot script:
 * \code
 * $ gnuplot p2p.plot
 * \endcode
 */

#include "config.h"
#include <getopt.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "benchmarks/mpib.h"
#include "p2p/mpib_p2p.h"




/*
* PRINT RESULTS IN TERMINAL
*/
void show_results(int rank, double time, int count, MPIB_result* results, double arr[]){

	if (rank == 0) {
		printf("#total benchmarking time: %le\n#\n", MPI_Wtime() - time);
		MPIB_print_result_th();
		int i;
		for (i = 0; i < count; i++){
			MPIB_print_result_tr(results[i]);
			arr[i] = results[i].T; // SAVE VALUES FOR POST-FORMULATE
		}
	}
	free(results);
}

/*
* APPLICATE FORMULA TCP RESULTS FOR T(1)
* 	( Ring1 / 2 ) - o1 - 2L0
*/

void time_adapt_tcp(MPIB_msgset msgset, MPIB_result* results, double arr_L1[], double arr_o1[],int count, double arr_L0[]){

	for(int p=0; p < count; p++)
		arr_L1[p] = (((arr_L1[p] / 2) - arr_o1[p]) - (2 * arr_L0[p]));
	
}


/*
* APPLICATE FORMULA INFINIBAND FOR T(1)
*	( Ring1 / 2 ) - o1
*/

void time_adapt_IB(MPIB_msgset msgset, MPIB_result* results, double arr_L1[], double arr_o1[],int count){

	for(int p=0; p < count; p++)
		arr_L1[p] = ((arr_L1[p] / 2) - arr_o1[p]);
	
}

/*
* APPLICATE FORMULA FOR T(0) 
*	(Ring0 - o0 / 2k)
*/
void time_adapt(MPIB_msgset msgset, MPIB_result* results,double arr_L0[], double arr_o0[],int count){

int k=1;

	for(int p=0; p < count;p++){

		arr_L0[p] = arr_L0[p] - arr_o0[p];
	
		arr_L0[p] = (arr_L0[p] / (2*k));
		
	}
}


int overhead_measure(int argc, char** argv){

	//MPIB_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm new_comm;
	MPI_Group group_world;
	int color;
	int exit;
	int num_procs;
	int p2p_type;
	int parallel = 0; /// not used at the moment
	int proc_num_spawned = 0;
	MPIB_getopt_help_default(&exit);
	MPIB_getopt_p2p_default(&parallel, &p2p_type, &proc_num_spawned);
	MPIB_msgset msgset;
	MPIB_getopt_msgset_default(&msgset);
	// if we want adaptive message sizes, we should do: msgset.stride = 0;
	MPIB_precision precision;
	MPIB_getopt_precision_default(&precision);
	int rank;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm,&num_procs);	
	if (rank == 0) {
		int c;
		char options[256] = "i:P:";
		strcat(options, MPIB_getopt_help_options());
		strcat(options, MPIB_getopt_msgset_options());
		strcat(options, MPIB_getopt_precision_options());
		strcat(options, MPIB_getopt_p2p_options());
		while ((c = getopt(argc, argv, options)) != -1) {
			MPIB_getopt_help_optarg(c, "p2p", &exit);
			MPIB_getopt_msgset_optarg(c, &msgset);
			MPIB_getopt_precision_optarg(c, &precision);
			MPIB_getopt_p2p_optarg(c, &parallel, &p2p_type, &proc_num_spawned);
		}
	}
	MPIB_getopt_help_bcast(&exit, 0, comm);
	if (exit) {
		MPI_Finalize();
		return 0;
	}
	MPIB_getopt_msgset_bcast(&msgset, 0, comm);
	MPIB_getopt_precision_bcast(&precision, 0, comm);
	MPIB_getopt_p2p_bcast(&parallel, &p2p_type, &proc_num_spawned, 0, comm);

	if (rank == 0)
		printf("#p2p adaptive benchmark 0-1\n#\n");
	MPIB_print_processors(comm);
	if (rank == 0) {
		MPIB_print_msgset(msgset);
		MPIB_print_precision(precision);
	}
	
	if (p2p_type == 1)
		p2p_init(comm, proc_num_spawned);

	
	MPIB_p2p_container* container = MPIB_roundtrip_container_alloc();
	int count = 0;
	double time = MPI_Wtime();
	
	// OVERHEAD EAGER
	if(rank == 0)
		printf("\n\n#OVERHEAD EAGER\n");
	MPIB_result* results_ov0 = NULL;
	MPIB_measure_overhead_eager(container, comm, 0, 1, msgset, precision, &count, &results_ov0);
	double arr_o0[count];
	show_results(rank,time,count,results_ov0,arr_o0);
	MPI_Barrier(comm);

	//OVERHEAD RENDEVOUZ
	if(rank == 0)
		printf("\n\n#OVERHEAD RENDEVOUZ\n");
	MPIB_result* results_ov1 = NULL;
	MPIB_measure_overhead_rdvz(container, comm, 0, 1, msgset, precision, &count, &results_ov1);
	double arr_o1[count];
	show_results(rank,time,count,results_ov1,arr_o1);
	MPI_Barrier(comm);
	
	if(msgset.comm_type == 0){
		//TRANSFER L0	
		double arr_L0[count];
		double arr_L0_copy[count];
		MPIB_result* results_t0 = NULL;		

		for( int reps=2; reps <= num_procs; reps++){
			if(rank == 0)
				printf("\n\n#TRANSFER L0 WITH %d PROCESSES\n",reps);


			color = rank/reps;
			MPI_Comm_split(comm,color,rank,&new_comm);
			MPI_Barrier(comm);

			//Solo procesos elegidos(1,2,3,..,n)
			if(color == 0){		
				MPI_Barrier(new_comm);
				MPIB_measure_transfer(container, new_comm, 0, 1, msgset, precision, &count, &results_t0,reps);
				MPI_Barrier(new_comm);
		
				if(rank == 0){
					for (int i = 0; i < count; i++){
						arr_L0[i] = results_t0[i].T;
						arr_L0_copy[i] = arr_L0[i]; // Para usarlo en L1
					}
				
				}

			MPI_Barrier(new_comm);
			if (rank == 0){
				time_adapt(msgset,results_t0,arr_L0,arr_o0,count);
			printf("arr after: %le\n", arr_L0[0]);
			}

			if(rank == 0)
				for (int i = 0; i < count; i++)
					results_t0[i].T = arr_L0[i];
		
			show_results(rank,time,count,results_t0,arr_L0);
			}

			//Esperamos por todos los procesos
			MPI_Barrier(comm);
			if(rank == 0)
				MPI_Comm_free(&new_comm);
		}
	
	}
	//TRANSFER L1	
	double arr_L1[count];
	MPIB_result* results_t1 = NULL;
	for( int reps=2; reps <= num_procs; reps++){
		if(rank == 0)
			printf("\n\n#TRANSFER L1 WITH %d PROCESSES\n",reps);


		color = rank/reps;
		MPI_Comm_split(comm,color,rank,&new_comm);
		MPI_Barrier(comm);
		printf("rank %d color %d procs %d\n", rank, color, reps);

		//Solo procesos elegidos(1,2,3,..,n)
		if(color == 0){		
			MPI_Barrier(new_comm);
			MPIB_measure_transfer(container, new_comm, 0, 1, msgset, precision, &count, &results_t1,reps);
			MPI_Barrier(new_comm);
		
			if(rank == 0){
				for (int i = 0; i < count; i++)
					arr_L1[i] = results_t1[i].T;
				

			}

		MPI_Barrier(new_comm);

//	FOR TCP USE time_adapt_tcp function, FOR INFINIBAND USE time_adapt_IB

		if (rank == 0)
			//time_adapt_tcp(msgset,results_t1,arr_L1,arr_o1,count,arr_L0_copy);	
			time_adapt_IB(msgset,results_t1,arr_L1,arr_o1,count);	

		if(rank == 0)
			for (int i = 0; i < count; i++)
				results_t1[i].T = arr_L1[i];
		
		show_results(rank,time,count,results_t1,arr_L1);
		}

		//Esperamos por todos los procesos
		MPI_Barrier(comm);
		if(rank == 0)
			MPI_Comm_free(&new_comm);
	}
	
		

	MPIB_roundtrip_container_free(container);
	
	if (p2p_type == 1) 
		p2p_finalize();
	
	return 0;

}


int main(int argc, char** argv) {

MPI_Init(&argc, &argv);

overhead_measure(argc, argv);

MPI_Finalize();
return 0;
	
}
