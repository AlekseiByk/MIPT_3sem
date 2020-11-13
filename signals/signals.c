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
    if (ret < 0)
    {
       perror ("make empty start mask error");
       exit (EXIT_FAILURE);
    }

    ret = sigaddset (&start_set, SIGUSR1);
    if (ret < 0)
    {
       perror ("add usr1 start mask error");
       exit (EXIT_FAILURE);
    }

    ret = sigaddset (&start_set, SIGUSR2);
    if (ret < 0)
    {
        perror ("add usr2 start mask error");
        exit (EXIT_FAILURE);
    }

    ret = sigaddset (&start_set, SIGCHLD);
    if (ret < 0)
    {
        perror ("add child start mask error");
        exit (EXIT_FAILURE);
    }

    ret = sigaddset (&start_set, SIGHUP);
    if (ret < 0)
    {
        perror ("add hup start mask error");
        exit (EXIT_FAILURE);
    }

    errno = 0;
    ret = sigprocmask (SIG_BLOCK, &start_set, NULL);
    if (ret < 0)
    {
        perror ("set start mask error");
        exit (EXIT_FAILURE);
    }

//**********************************************

	pid_t pid = fork ();
	if (pid < 0){
		perror ("Bad fork");
		exit (EXIT_FAILURE);
	}

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
    if (ret < 0)
    {
        perror ("sigfill reciever_usr error");
        exit (EXIT_FAILURE);
    }

    reciever_usr.sa_flags = 0;
    reciever_usr.sa_handler = handler;

	ret = sigaction (SIGUSR1, &reciever_usr, NULL);
	if (ret < 0)
    {
        perror ("sigaction SIGUSR1 error");
        exit (EXIT_FAILURE);
    }

	ret = sigaction (SIGUSR2, &reciever_usr, NULL);
	if (ret < 0)
    {
        perror ("sigaction SIGUSR2 error");
        exit (EXIT_FAILURE);
    }

//**********************************************

	struct sigaction child;

	ret = sigfillset (&child.sa_mask);
	if (ret < 0)
    {
        perror ("sigfill child mask error");
        exit (EXIT_FAILURE);
    }

    child.sa_flags = 0;
	child.sa_handler = sigchild;

	ret = sigaction (SIGCHLD, &child, NULL);
	if (ret < 0)
    {
        perror ("sigaction SIGCHLD error");
        exit (EXIT_FAILURE);
    }

//**********************************************

	sigset_t set;

	ret = sigfillset(&set);
	if (ret < 0)
    {
        perror ("sigfill parent set error");
        exit (EXIT_FAILURE);
    }

	ret = sigdelset(&set, SIGUSR1);
	if (ret < 0)
    {
        perror ("sigdelset error");
        exit (EXIT_FAILURE);
    }

	ret = sigdelset(&set, SIGUSR2);
	if (ret < 0)
    {
        perror ("sigdelset error");
        exit (EXIT_FAILURE);
    }

	ret = sigdelset(&set, SIGCHLD);
	if (ret < 0)
    {
        perror ("sigdelset error");
        exit (EXIT_FAILURE);
    }

//**********************************************

	while (1){
		char buffer = 0;

		for (int i = 0; i < 8; i++){

			errno = 0;
			ret = sigsuspend(&set);
			if (errno != EINTR)
		    {
		        perror ("sigsuspend for usr signals error");
		        exit (EXIT_FAILURE);
		    }

			if (transfer_bit){
				buffer += 1 << i;
			}

			errno = 0;
            int ret = kill(cpid, SIGUSR1);
            if (ret < 0)
            {
                perror("Bad sig to chld");
                exit(EXIT_FAILURE);
            }

		}

		write(1, &buffer, 1);
	}
}

//--------------------------------------------------------------------------

void child_func (char *filename)
{

	pid_t ppid =getppid();

	struct sigaction pause;

	errno = 0;
    int ret = sigfillset(&pause.sa_mask);
    if (ret < 0)
    {
        perror ("sigfillset pause error");
        exit (EXIT_FAILURE);
    }

    pause.sa_flags = 0;
    pause.sa_handler = child_handler;

	ret = sigaction (SIGUSR1, &pause, NULL);
	if (ret < 0)
    {
        perror ("sigaction pause error");
        exit (EXIT_FAILURE);
    }


//**********************************************


    struct sigaction hup;

	errno = 0;
    ret = sigfillset(&hup.sa_mask);
    if (ret < 0)
    {
        perror ("sigfillset hup error");
        exit (EXIT_FAILURE);
    }

    hup.sa_flags = 0;
    hup.sa_handler = child_hup;

	ret = sigaction (SIGHUP, &hup, NULL);
	if (ret < 0)
    {
        perror ("sigaction hup error");
        exit (EXIT_FAILURE);
    }

//**********************************************

	sigset_t set;
	ret = sigfillset(&set);
	if (ret < 0)
    {
        perror ("child`s sigfillset error error");
        exit (EXIT_FAILURE);
    }

	ret = sigdelset(&set, SIGUSR1);
	if (ret < 0)
    {
        perror ("sigdelset error");
        exit (EXIT_FAILURE);
    }

	ret = sigdelset(&set, SIGHUP);
	if (ret < 0)
    {
        perror ("sigdelset error");
        exit (EXIT_FAILURE);
    }

//**********************************************


	int input = open (filename, O_RDONLY);
	if (input < 0)
	{
		perror ("something wrong with file opening");
		exit (EXIT_FAILURE);
	}

//**********************************************

    ret = prctl(PR_SET_PDEATHSIG, SIGHUP);
    if (ret < 0)
    {
        perror("Set sig to parent death error");
        exit(EXIT_FAILURE);
    }

    if (old_ppid != getppid())
    {
        printf("No parent on start\n");
        exit(EXIT_FAILURE);
    }

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
			if (ret < 0)
			{
			    perror ("bad SIGUSR2 to parent");
			    exit (EXIT_FAILURE);
			}

			if (getppid() != ppid)
				exit (EXIT_FAILURE);

			errno = 0;
			ret = sigsuspend(&set);
			if (errno != EINTR)
		    {
		        perror ("suspend error");
		        exit (EXIT_FAILURE);
		    }
		}

	}
}

