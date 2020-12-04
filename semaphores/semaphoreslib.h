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
#include <fcntl.h>
#include <string.h>


const char* sem_file_name = "file";
const size_t buf_size = 4096;

void CheckError(int Expression, char * messege){
	if (Expression){
		perror (messege);
		exit (EXIT_FAILURE);
	}
}

enum {
	synchro		 = 0,
	write_ready	 = 1,
	read_ready	 = 2,
	alive_reader = 3,
	alive_writer = 4
};

#define SOPS(num, sem, op, flg)	do{\
									ops[num].sem_num = sem;\
									ops[num].sem_op  = op;\
									ops[num].sem_flg = flg;\
								}while(0);

