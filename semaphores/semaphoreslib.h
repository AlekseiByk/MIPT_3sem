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

const size_t buf_size = 4000;

char * make_fifo_name (pid_t pid)
{
	char * fifo = (char *) calloc (9, sizeof(char));

	sprintf (fifo, "%08d", pid);

	return fifo;
}
