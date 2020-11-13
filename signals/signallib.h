#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>


extern int transfer_bit;
extern int old_ppid;

void parent_func (pid_t cpid);
void child_func (char *filename);

void handler (int num);
void sigchild (int num);
void child_handler (int num);
void child_hup (int num);
