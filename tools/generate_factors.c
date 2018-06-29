/*!
 * \page generate_factors Generator of multiplying factors for benchmarking scatterv/gatherv
 * A standalone tool for generating factors which can be used as input file for benchmarking scatterv/gatherv in \ref collective.
 * The factors are normalized so that the average factor is 1. Arguments can be:
 * 	- -r for random factors
 * 	- -c for factors based on a small GSL matrix multiplication at every process
 *
 * \b Example:
 * \code mpirun -n 2 generate_factors -c > factors
 * \endcode
 *
 */
#include <getopt.h>
#include <string.h>
#include <time.h>

#include <gsl/gsl_blas.h>
#include <mpi.h>

#define ITER 10

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	int size;
	double*  speeds;
	int rank;
	double sum_speeds;
	int rflag=0,cflag=0; //rflag=1 for random, cflag=1 for CPU based
	int i;
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_size(comm, &size);
	MPI_Comm_rank(comm, &rank);

	
        if (rank == 0) {
                int c;
                while ((c = getopt(argc, argv,"hrc" )) != -1) {
                        switch (c) {
                                case 'h':
                                        fprintf(
                                                stderr,
"specify -r for a random factors generator or -c for a factor generator based on the computational power of the participating CPUs \n"
                                        );
                                        break;
                                case 'r':
					rflag=1;
                                        break;
                                case 'c':
					cflag=1;
                                        break;
                        }
                }
        }
	MPI_Bcast(&cflag, 1, MPI_INT, 0, comm);
	MPI_Bcast(&rflag, 1, MPI_INT, 0, comm);

	if (rflag){
		if (rank == 0 ){
			srand(time(NULL));
			speeds = (double*)malloc(sizeof(double) * size) ;
			for (i=0;i<size;i++){
				speeds[i]=(double)rand()/(double)RAND_MAX;
			}
		}
		
	}
	else if (cflag){
		int m = 100;
		gsl_matrix* A = gsl_matrix_alloc(m, m);
		gsl_matrix_set_all(A, 1);
		gsl_matrix* B = gsl_matrix_alloc(m, m);
		gsl_matrix_set_all(B, 2);
		gsl_matrix* C = gsl_matrix_alloc(m, m);
		double start, end, sum, speed;
		sum=0.;
		for (i=0;i<ITER;i++){
			start = MPI_Wtime();
			gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1, A, B, 0, C);
			end = MPI_Wtime();
			sum+= end - start;
		}
		sum/=ITER;
		speed = 1 / sum;
		speeds = rank == 0 ? (double*)malloc(sizeof(double) * size) : NULL;
		MPI_Gather(&speed, 1, MPI_DOUBLE, speeds, 1, MPI_DOUBLE, 0, comm);
		gsl_matrix_free(A);
		gsl_matrix_free(B);
		gsl_matrix_free(C);
	}
	if (rank == 0 && (rflag  || cflag)){
		sum_speeds=0.;
		for (i=0;i<size;i++){
			sum_speeds+=speeds[i];
		}
		for (i=0;i<size;i++){
			printf("%lf\n", (speeds[i] * size)/sum_speeds );
		}
		free(speeds);
	}

	MPI_Finalize();
	return 0;
}
