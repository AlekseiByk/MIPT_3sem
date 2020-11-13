#include <stdio.h>
#include "errno.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>


struct msgbuf {
    long mtype;
    char mtext[1];
};

long take_one_positive_number_from_args (int argc, char** argv);

int main(int argc, char **argv) 
{
	long input = take_one_positive_number_from_args (argc, argv);
	key_t key = ftok ("msg_childs.c", 0);
	int myid = -1; 

	int msgid = msgget (key, IPC_CREAT | 0666);

	if (msgid < 0){
	perror ("Error with msgget");
		return -1;
	}
	
	pid_t cpid = -1;
	for (int i = 0; i < input; i++)
	{
		cpid = fork();
		
		if (cpid == 0){
			myid = i + 1;
			break;
		}
	}

	if (cpid == 0)
	{
		struct msgbuf msg = {};
	    
		if (msgrcv (msgid,(void*) &msg, 1, myid, 0) == -1){
			perror ("error with msgrcv");
			return -1;
		}
		printf (" %d", myid);
		fflush (0);

		msg.mtype = myid + 1;
			
		if (msgsnd (msgid,(void*) &msg, 1, 0) == -1){
			perror ("Error with messenge sending");
			return -1;
		}

		return 0;
	}
	else{

		struct msgbuf msg = {};
		msg.mtype = 1;

		if (msgsnd (msgid,(void*) &msg, 1, 0) == -1){
			perror ("Error with messenge sending");
			return -1;
		}


		if (msgrcv (msgid,(void*) &msg, 1, input + 1, 0) == -1){
			perror ("error with msgrcv");
			return -1;
		}

		if (msgctl(msgid, IPC_RMID, 0) == -1){
			perror ("Error with msgctl");
			return -1;
		}

		printf ("\n");
	}
}






long take_one_positive_number_from_args (int argc, char** argv)
{
	char *strptr = NULL, *endptr = NULL;
	// Check argument's amount
	if (argc < 2) {
		printf("Too less arguments\n");
		return -1;
	} else if (argc > 2) {
		printf("Too many arguments\n");
		return -2;
	}
 
	// Check the string
	long input = 0;
	strptr = argv[1];
	input = strtol(strptr, &endptr, 10);
 
 
	if (endptr == strptr || *endptr != '\0') {
		printf("Wront input string\n");
		return -3;
	}
 
	if (input <= 0) {
		printf("The number must be greater then 0\n");
		return -4;
	}
 
	if (errno == ERANGE && (input == LONG_MAX || input == LONG_MIN)) {
		printf("Out of range\n");
		return -5;
	}

	return input;
}
