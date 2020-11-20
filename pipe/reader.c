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
	CheckError(ret < 0 && errno != EEXIST, "Creation transfer error(not EEXIST)\n");

    int service = open (fifo_service, O_WRONLY);
    CheckError(service < 0, "open service error");

	
	pid_t pid = getpid ();
	
	char * myfifo = make_fifo_name (pid);
	ret = mkfifo(myfifo, 0600);
	CheckError(ret == -1, "error with creating transfer fifo");
	
	int transfer = open(myfifo, O_RDONLY | O_NONBLOCK);
	CheckError(transfer < 0, "open transfer error");

	ret = write (service, myfifo, fifo_name_len);
	CheckError(ret == -1, "error with sending fifo name through service");

	close (service);

	ret = fcntl(transfer, F_SETFL, O_RDONLY);
	CheckError(ret == -1, "error with fcntl");

//*************************************

	fd_set rfds;
	struct timeval tv;

	FD_ZERO(&rfds);
	FD_SET(transfer, &rfds);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	errno = 0;

	if (select(transfer + 1, &rfds, NULL, NULL, &tv) <= 0){
		if (errno != 0)
			perror("select error");
		else 
			printf ("select timeout, file descriptor not availible");
		exit (EXIT_FAILURE);
	}
	
//*****************************************
	int count = -1;
	errno = 0;


	while (count != 0){
		
		count = read (transfer, buffer, buf_size);
		CheckError(count == -1, "read from fifo fail");

		count = write (1, buffer, count);
		CheckError(count == -1, "write to stdout failure");
	}

	close (transfer);
	free (buffer);


	ret = remove (myfifo);
	CheckError(ret == -1, "Remove error");

	free (myfifo);
	
	return 0;
}
