#include "fifolib.h"

int main (int argc, char **argv) 
{
	if ((argc < 2) || (argc > 2)){
		printf ("not right program start\n./writer <file>\n");
		exit (1);
	}

	errno = 0;

	char * buffer = (char *) calloc (buf_size, sizeof (buffer[0]));
	if (!buffer){
		printf ("error with buffer calloc");
		return 1;
	}

	int input = open (argv[1], O_RDONLY);
	CheckError(input < 0, "something wrong with file opening");

	int ret = mkfifo(fifo_service, 00600);
	CheckError(ret < 0 && errno != EEXIST, "Creation transfer error(not EEXIST)\n");

    
    int service = open (fifo_service, O_RDONLY);
    CheckError(service < 0, "error with transfer fifo open");

	char * fifo = (char *) calloc (fifo_name_len, sizeof(char));

	if (!fifo){
		printf ("error with fifo name calloc");
		return 1;
	}
	
	if (read (service, fifo, fifo_name_len) != fifo_name_len){
		perror ("Something wrong with pid transfer\n");
		exit (EXIT_FAILURE);
	}
	//sleep (5);

	int transfer = open (fifo, O_WRONLY | O_NONBLOCK);
	CheckError(transfer < 0, "error with transfer fifo open");

	close (service);

	ret = fcntl(transfer, F_SETFL, O_WRONLY);
	CheckError(ret == -1, "error with fcntl");

	free (fifo);
	//sleep(5);

	errno = 0;
	int count = 0;
	while ((count = read (input, buffer, buf_size)) > 0){
		CheckError(count == -1, "read from file error");

		count = write (transfer, buffer, count);
		CheckError(count <= 0 && errno == EPIPE, "Died transfer fifo");
		CheckError(count < 0, "write to fifo failure");

		//sleep(1);
	}

	close (transfer);
	close (input);
	free (buffer);
	return 0;
}
