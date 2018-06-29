#include "config.h"
#include "libmpib_coll.h"

#include <stdio.h>
#include <stdlib.h>

#include "benchmarks/mpib.h"

int mpib_coll_verbose = 0;

int mpib_coll_sgv = 0;

int collectives_initialize(MPI_Comm comm, char* subopts) {
	char* tokens[] = {"verbose", "sgv"};
	char* value;
	while (*subopts != '\0') {
		switch (getsubopt(&subopts, tokens, &value)) {
		case 0:
			mpib_coll_verbose = 1;
			break;
		case 1:
			mpib_coll_sgv = atoi(value);
			break;
		default:
			fprintf(stderr, "Unknown suboption %s\n", value);
			break;
		}
	}
	return 0;
}
