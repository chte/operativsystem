#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <stdio.h>			/* för kunna använda fget för indata från användaren */

int main(int argc, char **argv) {

	char word[70];
	int count = 1;

	/* Ta emot indata från användaren */
	while (fgets(word, sizeof(word), stdin) != NULL){
        count = strlen(word);
        word[--count] = '\0'; /*discard the newline character*/
        printf("Command to execute: %s %d\n",  word, count);

        if(strcmp(word,"exit") == 0){
       	 	printf("User want to exit, exit now\n");
       	 	break;
    	}
    	else {
    		executeForeGround(word);
    	}
    }

     exit(0);
}

void executeForeGround(char *cmd){
	pid_t pid;
	pid = fork();
	if (pid == 0){

    	/* Create the command */
		char* cmdLs[] = {&cmd,NULL};

		execvp("ls", cmdLs);
     	/*printf ("The value returned was: %d.\n",i);*/
		printf("Unknown command\n");

		exit(0);
	}
	else{
		int status = 0;
		int childId = wait(&status);
		printf ("Child process(id %d) is returned with status: %d.\n",childId,status);
	}

	return;
}