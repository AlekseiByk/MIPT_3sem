#include "signallib.h"

int transfer_bit = 0;
int old_ppid = 0;

int main (int argc, char ** argv) 
{
	if ((argc < 2) || (argc > 2)){
		printf ("not right program start\n./sygnal <file>\n");
		exit (1);
	}

	old_ppid = getpid();

//**********************************************

	errno = 0;
    sigset_t start_set;

    int ret = sigemptyset (&start_set);
    CheckError(ret < 0, "make empty start mask error");

    ret = sigaddset (&start_set, SIGUSR1);
    CheckError(ret < 0, "add usr1 start mask error");

    ret = sigaddset (&start_set, SIGUSR2);
    CheckError(ret < 0, "add usr2 start mask error");

    ret = sigaddset (&start_set, SIGCHLD);
    CheckError(ret < 0, "add chld start mask error");

    ret = sigaddset (&start_set, SIGHUP);
    CheckError(ret < 0, "add hup start mask error");

    errno = 0;
    ret = sigprocmask (SIG_BLOCK, &start_set, NULL);
    CheckError(ret < 0, "set start mask error");

//**********************************************

	pid_t pid = fork ();
    CheckError(ret < 0, "Bad fork");

	if (pid > 0)
		// parent
		parent_func (pid);
		
	
	else
		//child
		child_func (argv[1]);

	return 0;
}

//------------------------------------------------------------------------

void handler (int num){

    if (num == SIGUSR1)
        transfer_bit = 1;
    else
        transfer_bit = 0;
    return;
}

void sigchild (int num){
    printf ("Parent has recieved SIGCHLD\n");
    exit (EXIT_FAILURE);
}

void child_handler (int num){
    return;
}

void child_hup (int num){
    if (old_ppid != getppid())
        printf("child recieved SIGHUP(parent has died)");
    exit (EXIT_FAILURE);
}

//------------------------------------------------------------------------

void parent_func (pid_t cpid){

	errno = 0;
	struct sigaction reciever_usr;

    int ret = sigfillset (&reciever_usr.sa_mask);
    CheckError(ret < 0, "sigfill reciever_usr error");


    reciever_usr.sa_flags = 0;
    reciever_usr.sa_handler = handler;

	ret = sigaction (SIGUSR1, &reciever_usr, NULL);
    CheckError(ret < 0, "sigaction SIGUSR1 error");

	ret = sigaction (SIGUSR2, &reciever_usr, NULL);
    CheckError(ret < 0, "sigaction SIGUSR2 error");

//**********************************************

	struct sigaction child;

	ret = sigfillset (&child.sa_mask);
    CheckError(ret < 0, "sigfill child mask error");

    child.sa_flags = 0;
	child.sa_handler = sigchild;

	ret = sigaction (SIGCHLD, &child, NULL);
    CheckError(ret < 0, "sigaction SIGCHLD error");

//**********************************************

	sigset_t set;

	ret = sigfillset(&set);
    CheckError(ret < 0, "sigfill parent set error");

	ret = sigdelset(&set, SIGUSR1);
    CheckError(ret < 0, "sigdelset error");

	ret = sigdelset(&set, SIGUSR2);
    CheckError(ret < 0, "sigdelset error");

	ret = sigdelset(&set, SIGCHLD);
    CheckError(ret < 0, "sigdelset error");

//**********************************************

	while (1){
		char buffer = 0;

		for (int i = 0; i < 8; i++){

			errno = 0;
			ret = sigsuspend(&set);
            CheckError(errno != EINTR, "sigsuspend for usr signals error");

			if (transfer_bit){
				buffer += 1 << i;
			}

			errno = 0;
            int ret = kill(cpid, SIGUSR1);
            CheckError(ret < 0, "kill to child error");

		}

		ret = write(1, &buffer, 1);
        CheckError(ret < 0, "write to stdout error");
	}
}

//--------------------------------------------------------------------------

void child_func (char *filename)
{

	pid_t ppid =getppid();

	struct sigaction pause;

	errno = 0;
    int ret = sigfillset(&pause.sa_mask);
    CheckError(ret < 0, "sigfillset pause error");

    pause.sa_flags = 0;
    pause.sa_handler = child_handler;

	ret = sigaction (SIGUSR1, &pause, NULL);
    CheckError(ret < 0, "sigaction pause error");


//**********************************************


    struct sigaction hup;

	errno = 0;
    ret = sigfillset(&hup.sa_mask);
    CheckError(ret < 0, "sigfillset hup error");

    hup.sa_flags = 0;
    hup.sa_handler = child_hup;

	ret = sigaction (SIGHUP, &hup, NULL);
    CheckError(ret < 0, "sigaction hup error");

//**********************************************

	sigset_t set;
	ret = sigfillset(&set);
    CheckError(ret < 0, "child`s sigfillset error error");

	ret = sigdelset(&set, SIGUSR1);
    CheckError(ret < 0, "sigdelset error");

	ret = sigdelset(&set, SIGHUP);
    CheckError(ret < 0, "sigdelset error");

//**********************************************

	int input = open (filename, O_RDONLY);
    CheckError(ret < 0, "something wrong with file opening");

//**********************************************

    ret = prctl(PR_SET_PDEATHSIG, SIGHUP);
    CheckError(ret < 0, "Set sig to parent death error");

    CheckError(old_ppid != getppid(), "No parent on start");

	char buffer = 0;

	while (read(input, &buffer, 1) != 0)
	{

		for (int i = 0; i < 8; i++)
		{

			errno = 0;

			if ((((int)buffer >> i) & 1) == 1)
				ret = kill (ppid, SIGUSR1);
			else
				ret = kill (ppid, SIGUSR2);
            CheckError(ret < 0, "bad SIGUSR to parent");

			if (getppid() != ppid)
				exit (EXIT_FAILURE);

			errno = 0;
			ret = sigsuspend(&set);
			CheckError(errno != EINTR, "sigsuspend error");
		}

	}
}

