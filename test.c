#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#include<error.h>
#include<fcntl.h>

int main()
{
	int in,out;
	char *argv[] = {"sort", NULL};
	in = open("makefile", O_RDONLY);
	dup2(in,STDIN_FILENO);
	close(in);
	out = open("file2",O_WRONLY|O_CREAT|O_APPEND,0666); // Should also be symbolic values for access rights
	dup2(out,STDOUT_FILENO);
	close(out);
	execvp(argv[0], argv);
	close(1);
		
}
