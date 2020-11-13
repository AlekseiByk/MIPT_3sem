#include <errno.h>
#include <sys/select.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


const char * fifo_service = "chat.fifo";
const int fifo_name_len = 15;
const size_t buf_size = 4000;

char * make_fifo_name (pid_t pid)
{
	char * fifo = (char *) calloc (15, sizeof(char));

	sprintf (fifo, "fifos/%08d", pid);

	return fifo;
}


void CheckError(int Expression, char * messege){
	if (Expression){
		perror (messege);
		exit (EXIT_FAILURE);
	}
}
