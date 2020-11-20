#include "semaphoreslib.h"
 
int main() 
{
	int ret = open (sem_file_name, IPC_CREAT | 0666);
	CheckError (ret == -1, "open key file error");
	
	key_t semkey = ftok (sem_file_name, 0);

	errno = 0;
	int semid = semget (semkey, 5, IPC_CREAT);
	CheckError (semid == -1, "Semget error");

	int shmid = shmget (semkey, buf_size, IPC_CREAT);
	CheckError (shmid == -1, "Shmget error");

	void *buffer = shmat (shmid, NULL, SHM_RDONLY);
	CheckError (buffer == -1, "Shmat error");



	int ret = shmdt(buffer);
	CheckError (ret == -1, "Shmdt error");
	return 0;
}
