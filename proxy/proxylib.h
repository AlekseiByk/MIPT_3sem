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

struct pipes_info
{
	int from_parent_fd[2];		//reciever pipe
	int to_parent_fd[2];		//transmitter pipe
	int child_pid;			//for wait
};

struct info{
	int    fd_wr;
    int    fd_rd;
    size_t buff_size;
    char*  write_end;
    char*  read_end;
    char*  buff;
    char*  end;
    size_t empty;
    size_t full;
};

int child_func (const char * filename, pipes_info* links, int number, int children_count);
int parent_func (pipes_info* links, int children_count, int number_of_fd);

void CheckError(int Expression, const char * messege){
	if (Expression){             
		perror (messege);
		exit (EXIT_FAILURE);
	}
}

long take_one_positive_number_from_args (int argc, char** argv)
{
	char *strptr = NULL, *endptr = NULL;
 
	// Check the string
	long input = 0;
	strptr = argv[1];
	input = strtol(strptr, &endptr, 10);
 
 
	if (endptr == strptr || *endptr != '\0') {
		printf("Wront input string\n");
		return -3;
	}
 
	if (input <= 0) {
		printf("The number must be greater then 0\n");
		return -4;
	}
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		return -5;
	}

	return input;
}