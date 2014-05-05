#include <stdlib.h>
#include <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>

int main(int argc, char **argv) {

	pid_t  pid;

	pid = fork();
    if (pid == 0){

    	/* Create the command */
     	char* cmdLs[] = {"ls",NULL};

     	execvp("ls", cmdLs);
     	/*printf ("The value returned was: %d.\n",i);*/
     	printf("Unknown command\n");

    	exit(0);
    }
    else{
     	int status = 0;
     	wait(&status);
     	printf ("Child process is returned with: %d.\n",status);
    }


     exit(0);
 }