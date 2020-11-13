#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>


extern int transfer_bit;
extern int old_ppid;

void handler (int num){

	if (num == SIGUSR1)
		transfer_bit = 1;
	else
		transfer_bit = 0;
	return;
}

void sigchild (int num){
	printf ("Parent has recieved SIGCHLD\n");
	exit (EXIT_FAILURE);
}

void child_handler (int num){
	return;
}

void child_hup (int num){
	if (oldppid != getppid())
		printf("child recieved SIGHUP(parent has died)");
	exit (EXIT_FAILURE);
}
