#include "semaphoreslib.h"

/*
 enum {
	synchro		 = 0,
	write_ready	 = 1,
	read_ready	 = 2,
	alive_reader = 3,
	alive_writer = 4,
};
*/

int main()
{
	errno = 0;
	int ret = open (sem_file_name, O_CREAT | O_RDONLY, 0666);
	CheckError (ret == -1  && errno != EEXIST, "open key file error");

	key_t semkey = ftok (sem_file_name, 0);

	close (ret);

	errno = 0;
	int semid = semget (semkey, 5, IPC_CREAT | 0666);
	CheckError (semid == -1, "Semget error");

	int shmid = shmget (semkey, buf_size, IPC_CREAT | 0666);
	CheckError (shmid == -1, "Shmget error");

	void *sh_mem = shmat ( shmid, NULL, SHM_RDONLY);
	CheckError (sh_mem == NULL, "Shmat error");

	//************************************************
	
	struct sembuf ops[5] = {};

	SOPS (0, alive_reader, 0, IPC_NOWAIT);
	SOPS (1, alive_reader, 1, SEM_UNDO);

	ret = semop (semid, ops, 2);
	CheckError (ret == -1 && errno != EAGAIN, "semop error");

	if (ret == -1 && errno == EAGAIN){
		printf ("reader is already existing in the system\n");
		exit (1);
	}

	SOPS (0, synchro, 0, 0);
	ret = semop (semid, ops, 1);
	CheckError (ret == -1, "semop error");


	semctl(semid, write_ready, SETVAL, 2);

	SOPS (0, write_ready, -1, SEM_UNDO);
	SOPS (1, alive_reader, 1, SEM_UNDO);

	ret = semop (semid, ops, 2);
	CheckError (ret == -1, "semop error");

	SOPS (0, alive_writer, -2, SEM_UNDO);
	SOPS (1, alive_writer, 2, SEM_UNDO);
	SOPS (2, synchro, 1, SEM_UNDO);
	//SOPS (3, end, 1, SEM_UNDO);

	ret = semop (semid, ops, 3);
	CheckError (ret == -1, "semop error");

	//-------------------------------------------------

	int read_val = 0;

	do{

		SOPS (0, read_ready, -1, 0);
		ret = semop (semid, ops, 1);
		CheckError (ret == -1, "semop error");

		if (semctl (semid, synchro, GETVAL) != 2){
			printf ("writer has suddenly died\n");
			exit (1);
		}

		read_val = *((int *) sh_mem);

		write (1, sh_mem + sizeof (int), read_val);

		SOPS (0, write_ready, 1, 0);
		ret = semop (semid, ops, 1);
		CheckError (ret == -1, "semop error");

	}while(read_val == buf_size - sizeof(int));

	//*************************************************

	ret = shmdt(sh_mem);
	CheckError (ret == -1, "Shmdt error");
	return 0;
}
