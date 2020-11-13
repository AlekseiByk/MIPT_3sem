#include "fifolib.h"
 
int main() 
{
	char * buffer = (char *) calloc (buf_size, sizeof (buffer[0]));

	int service = open ( fifo_service, O_WRONLY);

	if (service == -1){
		mkfifo ( fifo_service, 0666);
		service = open ( fifo_service, O_WRONLY);
	}

	pid_t pid = getpid ();
	
	char * myfifo = make_fifo_name (pid);
	mkfifo(myfifo, 0600);
	
	int transfer = open(myfifo, O_RDONLY | O_NONBLOCK);
	if (transfer < 0){
		printf ("error with transfer fifo open");
		return 1;
	}
	write (service, myfifo, fifo_name_len);

	free (myfifo);
	fcntl(transfer, F_SETFD, ~O_NONBLOCK);


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

	int count = 0;
	while ((count = read (transfer, buffer, buf_size)) != 0){
		write (1, buffer, count);
	}


	close (service);
	close (transfer);
	free (buffer);
	return 0;
}
