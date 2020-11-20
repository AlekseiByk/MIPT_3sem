#include <errno.h>
#include <sys/select.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

const char* sem_file_name = "reader";
const size_t buf_size = 4000;

void CheckError(int Expression, char * messege){
	if (Expression){
		perror (messege);
		exit (EXIT_FAILURE);
	}
}


