#include "fifolib.h"
 
int main() 
{
	char * buffer = (char *) calloc (buf_size, sizeof (buffer[0]));
	if (!buffer){
		printf ("error with buffer calloc");
		return 1;
	}

	errno = 0;
	int ret = mkfifo(fifo_service, 00600);
    if (ret < 0 && errno != EEXIST)
    {
      perror("Creation transfer error(not EEXIST)\n");
      exit(EXIT_FAILURE);
    }
    
    int service = open (fifo_service, O_WRONLY);
    if (service < 0){
		printf ("error with transfer fifo open");
		return 1;
	}
	
	pid_t pid = getpid ();
	
	char * myfifo = make_fifo_name (pid);
	ret = mkfifo(myfifo, 0600);
	if (ret == -1){
		perror("error with creating transfer fifo");
		exit (EXIT_FAILURE);
	}
	
	int transfer = open(myfifo, O_RDONLY | O_NONBLOCK);
	if (transfer < 0){
		perror ("error with transfer fifo open");
		exit (EXIT_FAILURE);
	}

	ret = write (service, myfifo, fifo_name_len);
	if (ret == -1){
		perror("error with sending fifo name through service");
		exit (EXIT_FAILURE);
	}
	sleep(5);

	free (myfifo);

	ret = fcntl(transfer, F_SETFD, ~O_NONBLOCK);
	if (ret == -1){
		perror("error with fcntl");
		exit (EXIT_FAILURE);
	}

//*************************************
	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(transfer, &rfds);

	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	if (select(transfer + 1, &rfds, NULL, NULL, &tv) <= 0){
		printf ("select timeout, file descriptor not availible");
		exit (1);
	}
//*****************************************
	int count = 0;
	errno = 0;
	while ((count = read (transfer, buffer, buf_size)) != 0){

		if (count == -1){
			perror("read from fifo fail");
			exit(EXIT_FAILURE);
		}

		count = write (1, buffer, count);
		if (count == -1){
			perror("write to stdout failure");
			exit(EXIT_FAILURE);
		}
	}


	close (service);
	close (transfer);
	free (buffer);
	return 0;
}
