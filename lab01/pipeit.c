#include <stdio.h>	// provides printf
#include <fcntl.h>	// provides constants O_CREAT and O_WRONLY
#include <unistd.h> 	// provides pipe, fork, dup2, and execlp
#include <stdlib.h>	// provides exit

int checkSysCall(int returnVal);

int main()
{
	int fd[2], status;
	pid_t pid;

	pipe(fd);

	if ((pid = fork()) < 0)
	{
		printf("fork failed\n");
		return -1;
	}
	else if (pid == 0)
	{
		// child 1: sort -r > outfile
		checkSysCall(close(fd[1]));					/* child 1 will not write to pipe  */
		checkSysCall(dup2(fd[0], STDIN_FILENO));	/* stdin now points to the read-end of the pipe */
		checkSysCall(close(fd[0]));					/* file descriptor is not needed anymore */
		
		char *filename = "outfile";
		int fw;

		if ((fw = open("outfile", O_CREAT|O_RDWR)) == -1)
		{
			perror(filename);
			exit(EXIT_FAILURE);
		}

		checkSysCall(dup2(fw, STDOUT_FILENO));					/* stdout now points to file descriptor of outfile */
		checkSysCall(close(fw));									/* file descriptor not needed anymore */
		checkSysCall(execlp("sort", "sort", "-r", NULL));	/* executes sort -r > outfile */
	}
	
	// parent: creates another child process
	pid_t pid2;

	if ((pid2 = fork()) < 0)
	{
		printf("fork failed\n");
		return -1;
	}
	else if (pid2 == 0)
	{
		// child 2: ls
		checkSysCall(close(fd[0]));						/* child 2 will not read from pipe */
		checkSysCall(dup2(fd[1], STDOUT_FILENO));		/* stdout now points to the write-end of the pipe */
		checkSysCall(close(fd[1]));						/* file descriptor is not needed anymore */
		checkSysCall(execlp("ls", "ls", NULL));		/* execute ls */
	}
		
	checkSysCall(close(fd[0]));
	checkSysCall(close(fd[1]));

	if (waitpid(pid, &status, 0) == -1)
	{
		printf("error with child 1\n");
	}

	return 0;
}

int checkSysCall(int returnVal)
{
	if (returnVal == -1)
	{
		perror(NULL);
		exit(EXIT_FAILURE);
	}
	return returnVal;
}
