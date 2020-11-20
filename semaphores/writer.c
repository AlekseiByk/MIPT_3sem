#include "semaphoreslib.h"
 
int main (int argc, char **argv) 
{
	if ((argc < 2) || (argc > 2)){
		printf ("not right program start\n%s <file>\n", argv[0]);
		exit (1);
	}

	key_t semkey = ftok (sem_file_name, 0);

	errno = 0;
	int semid = semget (semkey, 5, IPC_CREAT);
	CheckError (semid == -1, "Semget error");

	void *buffer = shmat (shmid, NULL, 0);
	CheckError (buffer == -1, "Shmat error");

	

	int ret = shmdt(buffer);
	CheckError (ret == -1, "Shmdt error");

	return 0;
}
