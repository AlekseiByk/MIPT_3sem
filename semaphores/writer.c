#include "fifolib.h"
 
int main (int argc, char **argv) 
{
	if ((argc < 2) || (argc > 2)){
		printf ("not right program start\n./writer <file>\n");
		exit (1);
	}

	char * buffer = (char *) calloc (buf_size, sizeof (buffer[0]));

	int input = open (argv[1], O_RDONLY);
	if (input < 0){
		printf ("something wrong with file opening");
		exit (1);
	}

	int service = open ( fifo_service, O_RDONLY);
	if (service < 0){
		mkfifo ( fifo_service, 0666);
		service = open ( fifo_service, O_RDONLY);
	}
	printf("%d\n", service);

	char * fifo = (char *) calloc (9, sizeof(char));
/*
	
*/
	if (read (service, fifo, 9) != 9){
		printf ("Something wrong with pid transfer\n");
		exit (1);
	}

	int transfer = open (fifo, O_WRONLY );
	if (transfer < 0){
		printf ("error with transfer fifo open");
		return 1;
	}

	free (fifo);

	printf("work\n");
	fflush(0);

	int count = 0;
	while ((count = read (input, buffer, buf_size)) != 0){
		write (transfer, buffer, count);
		sleep(1);
	}


	close (service);
	close (transfer);
	close (input);
	free (buffer);
	return 0;
}
