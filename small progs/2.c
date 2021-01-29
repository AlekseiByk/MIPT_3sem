#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


int proc();

int main(int argc, char** argv)
{
	if (argc < 2) 
	{
		printf("Too less arguments\n");
		return -1;
	} 
	
	execvp(argv[1], &(argv[1]));
	
	return 0;
}
