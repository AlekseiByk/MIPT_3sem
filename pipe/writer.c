#include "fifolib.h"

int main (int argc, char **argv) 
{
	if ((argc < 2) || (argc > 2)){
		perror ("not right program start\n./writer <file>\n");
		exit (1);
	}

	errno = 0;

	char * buffer = (char *) calloc (buf_size, sizeof (buffer[0]));
	if (!buffer){
		printf ("error with buffer calloc");
		return 1;
	}

	int input = open (argv[1], O_RDONLY);
	if (input < 0){
		perror ("something wrong with file opening");
		exit (1);
	}

	int ret = mkfifo(fifo_service, 00600);
    if (ret < 0 && errno != EEXIST)
    {
		perror("Creation transfer error(not EEXIST)\n");
		exit(EXIT_FAILURE);
    }
    
    int service = open (fifo_service, O_RDONLY);
    if (service < 0){
		perror ("error with transfer fifo open");
		return 1;
	}

	printf("%d\n", service);

	char * fifo = (char *) calloc (fifo_name_len, sizeof(char));
	if (!fifo){
		printf ("error with fifo name calloc");
		return 1;
	}
	
	if (read (service, fifo, fifo_name_len) != fifo_name_len){
		perror ("Something wrong with pid transfer\n");
		exit (EXIT_FAILURE);
	}

	int transfer = open (fifo, O_WRONLY | O_NONBLOCK);
	if (transfer < 0){
		perror ("error with transfer fifo open");
		return 1;
	}

	ret = fcntl(transfer, F_SETFL, O_WRONLY);
	if (ret == -1){
		perror("error with fcntl");
		exit (EXIT_FAILURE);
	}

	free (fifo);
	sleep(5);

	errno = 0;
	int count = 0;
	while ((count = read (input, buffer, buf_size)) > 0){
		if (count == -1){
			perror("read from file error");
			exit(EXIT_FAILURE);
		}

		count = write (transfer, buffer, count);
		if (count <= 0 && errno == EPIPE){
			perror("Died transfer fifo");
			exit(EXIT_FAILURE);
		}
		if (count < 0){
			perror("write to fifo failure");
			exit(EXIT_FAILURE);
		}
		sleep(1);
	}


	close (service);
	close (transfer);
	close (input);
	free (buffer);
	return 0;
}
