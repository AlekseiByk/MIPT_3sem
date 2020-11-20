#include "semaphoreslib.h"
 
int main (int argc, char **argv) 
{
	key_t key ftok (sem_file_name, 0);
	semget (semkey, 5, IPC_CREAT | IPC_EXCL);
	

	return 0;
}
