#include "proxylib.h"

const size_t Chld_buff_size = 128000;
const size_t Max_buff_size = 128000;

enum {
	RD_PIPE = 0,
	WR_PIPE = 1,
};

int main (int argc, char ** argv)
{
	if ((argc < 3) || (argc > 3)){
		printf ("not right program start\n%s <number of children> <file>\n", argv[0]);
		exit (1);
	}
	int children_count = take_one_positive_number_from_args (argc, argv);

	struct pipes_info* links = (pipes_info*) calloc (children_count, sizeof(links[0]));
	CheckError (links == NULL, "Bad calloc");

	int max_fd = 0; //for select
	int ret    = 0;
	int i = 0;
	pid_t child_pid = 0;
	errno = 0;

	while (i < children_count)
	{
		if ( i != children_count - 1) {	//not the last child (there is pipe to parent)
			ret = pipe2 (links[i].to_parent_fd, O_NONBLOCK);
			CheckError (ret == -1, "pipe to parent creation error");

			if (links[i].to_parent_fd[0] > max_fd)
				max_fd = links[i].to_parent_fd[0];
			if (links[i].to_parent_fd[1] > max_fd)
				max_fd = links[i].to_parent_fd[1];
		}

		if ( i != 0) {	//not the first child (there is pipe from parent)
			ret = pipe2 (links[i].from_parent_fd, O_NONBLOCK);
			CheckError (ret == -1, "pipe from parent creation error");

			if (links[i].from_parent_fd[0] > max_fd)
				max_fd = links[i].from_parent_fd[0];
			if (links[i].from_parent_fd[1] > max_fd)
				max_fd = links[i].from_parent_fd[1];
		}

		child_pid = fork ();
		CheckError (child_pid == -1, "fork error");

		if (child_pid == 0) {			//child

			for(int j = 0; j < i; j++)
				close (links[j].from_parent_fd[WR_PIPE]);

			child_func (argv[2], links + i, i, children_count);
			return 0;
		}

		else {							//parent

			links[i].child_pid = child_pid;

			if (i != 0)
				close (links[i].from_parent_fd[RD_PIPE]);		//because parent only writes into this pipe

			if (i != children_count - 1)
				close (links[i].to_parent_fd[WR_PIPE]);			//because parent only reads from this pipe
		}

		i++;
	}

	parent_func (links, children_count, max_fd + 1);

	return 0;
}

int child_func (const char * filename, pipes_info* links, int number, int children_count)
{

	int ret = 0;

	if (number > 0)
		close (links->from_parent_fd[WR_PIPE]);
	if (number < children_count - 1)
		close (links->to_parent_fd[RD_PIPE]);

	errno = 0;
	if (number == 0){								//the first reads from file
		links->from_parent_fd[RD_PIPE] = open (filename, O_RDONLY);
		CheckError (links->from_parent_fd[RD_PIPE] == -1, "open file error");
	}

	if (number == children_count - 1)				//the last writes into STDOUT
		links->to_parent_fd[1] = STDOUT_FILENO;

	if (number != 0) {
		ret = fcntl (links->from_parent_fd[RD_PIPE], F_SETFL, O_RDONLY);
		CheckError (ret == -1, "fcntl error");
	}

	if (number != number - 1) {
		ret = fcntl (links->to_parent_fd[WR_PIPE], F_SETFL, O_WRONLY);
		CheckError (ret == -1, "fcntl error");
	}

	char* buff = (char*) calloc (Chld_buff_size, sizeof(char));
	CheckError (buff == NULL, "child can't allocate buffer");

	ssize_t ret_read = 0;
    do {
    	
        ret_read = read(links->from_parent_fd[0], buff, Chld_buff_size);
        CheckError (ret_read == -1, "read from pipe error(child)");

        int ret_write = write(links->to_parent_fd[1], buff, ret_read);
        CheckError (ret_write == -1, "write to pipe error(child)");

    } while(ret_read > 0);

	close (links->from_parent_fd[RD_PIPE]);
	close (links->to_parent_fd[WR_PIPE]);
	free (buff);
	exit (EXIT_SUCCESS);
}

int parent_func (pipes_info* links, int children_count, int fd_max)
{
	errno = 0;
	info* nodes = (info*) calloc (children_count - 1, sizeof(nodes[0]));
	CheckError (nodes == NULL, "parent *nodes calloc error");

	for (int i = 0; i < children_count - 1; i++)
	{

		nodes[i].buff_size = 1024;
		for (long j = 0; j < children_count - 2 - i; j++)
			if (!(nodes[i].buff_size > Max_buff_size / 3))
				nodes[i].buff_size *= 3;
			else
				break;

		errno = 0;
		nodes[i].buff = (char*) calloc (nodes[i].buff_size, sizeof(nodes[i].buff[0]));
		CheckError (nodes[i].buff == NULL, "Alloc info buff error");

		nodes[i].end       = nodes[i].buff + nodes[i].buff_size;
		nodes[i].write_end = nodes[i].buff;
		nodes[i].read_end  = nodes[i].buff;

		nodes[i].empty     = nodes[i].buff_size;
		nodes[i].full      = 0;

		nodes[i].fd_rd     = links[i].to_parent_fd[RD_PIPE];
		nodes[i].fd_wr     = links[i + 1].from_parent_fd[WR_PIPE];
	}

	int first_alive = 0;
	do
	{
		fd_set readfds,writefds;

		FD_ZERO (&readfds);
		FD_ZERO (&writefds);

		for (long int i = first_alive; i < children_count - 1; i++)
		{
			if (nodes[i].fd_rd >= 0 && nodes[i].empty > 0)
				FD_SET (nodes[i].fd_rd, &readfds);

			if (nodes[i].fd_wr >= 0 && nodes[i].full > 0)
				FD_SET (nodes[i].fd_wr, &writefds);
		}

		errno = 0;
		int num_ready = select (fd_max, &readfds, &writefds, NULL, NULL);
		CheckError (num_ready < 0 && errno != EINTR, "select error");


		for(long int i = first_alive; i < children_count - 1; i++)
		{

			if (FD_ISSET (nodes[i].fd_rd, &readfds) && nodes[i].empty > 0)
			{
				errno = 0;
				ssize_t ret_read = read (nodes[i].fd_rd, nodes[i].read_end, sizeof(nodes[i].buff[0]) * nodes[i].empty);
				CheckError (ret_read == -1, "read from pipe error");

				if (ret_read == 0)
				{
					close (nodes[i].fd_rd);
					nodes[i].fd_rd = -1;
				}

				if (nodes[i].read_end + ret_read == nodes[i].end)
				{
					nodes[i].read_end = nodes[i].buff;
					nodes[i].full += ret_read;
					nodes[i].empty = nodes[i].write_end - nodes[i].read_end;
				}
				else
				{
					if (nodes[i].read_end >= nodes[i].write_end)
						nodes[i].full += ret_read;
					nodes[i].empty -= ret_read;
					nodes[i].read_end += ret_read;
				}
			}

			if (FD_ISSET (nodes[i].fd_wr, &writefds) && nodes[i].full > 0)
			{
				errno = 0;
				ssize_t ret_write = write (nodes[i].fd_wr, nodes[i].write_end, sizeof(nodes[i].buff[0]) * nodes[i].full);
				CheckError (ret_write == -1, "write to pipe error");

				if (nodes[i].write_end + ret_write == nodes[i].end)
				{
					nodes[i].write_end = nodes[i].buff;
					nodes[i].empty += ret_write;
					nodes[i].full = nodes[i].read_end - nodes[i].write_end;
				}
				else
				{
					if (nodes[i].write_end >= nodes[i].read_end)
						nodes[i].empty += ret_write;
					nodes[i].full -= ret_write;
					nodes[i].write_end += ret_write;
				}
			}

			if (nodes[i].fd_rd == -1 && nodes[i].full == 0 && nodes[i].fd_wr != -1)
			{
				close (nodes[i].fd_wr);
				nodes[i].fd_wr = -1;

				if (first_alive != i)
				{
					printf ("Child %ld was killed\n", i);
					fflush (0);
					exit (EXIT_FAILURE);
				}

				first_alive++;
				free (nodes[i].buff);
			}
		}
	} while (first_alive < children_count - 1);

	for (int i = 0; i < children_count - 1; i++)
	{
	    if (nodes[i].fd_wr != -1)
	        close (nodes[i].fd_wr);
	    if (nodes[i].fd_rd != -1)
	        close (nodes[i].fd_rd);
	}


	for (int i = 0; i < children_count; i++)
	    {
	        errno  = 0;
	        int ret = waitpid (links[i].child_pid, NULL, 0);
	        CheckError (ret == -1, "waitpid error");
	    }

	exit (EXIT_SUCCESS);
}