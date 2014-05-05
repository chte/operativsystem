#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <stdio.h>			/* för kunna använda fget för indata från användaren */
#include <string.h>         /* definierar strings */


void executeForeGround(char**);

int main(int argc, char **argv) {

	char word[70];
	int count = 1;

	/* Skriv ut en välkomns text som beskriver vad man kan göra */
	printf("Welcome to miniShell!\n");
	printf("    Build in commands:\n");
	printf("     - <any command supported by your OS, i.e ls>\n");
	printf("     - cd <folder path>\n");
	printf("     - exit\n");


	/* Ta emot indata från användaren */
	while (fgets(word, sizeof(word), stdin) != NULL){
        count = strlen(word);
        word[--count] = '\0'; /*discard the newline character*/
        /*printf("Command to execute: %s %d\n",  word, count);*/

        /* Om användaren skrev in exit vill den sluta så gör en break */
        if(strcmp(word,"exit") == 0){
       	 	printf("Exiting...\n");
       	 	break;
    	}
    	else {
    		char *cmd[] = {word,NULL};
    		executeForeGround(cmd);
    	}
    }

     exit(0);
}

void executeForeGround(char **cmd){
	pid_t pid;
	pid = fork();
	if (pid == 0){
		execvp(cmd[0], cmd);
     	
		printf("Unknown command\n");
	}
	else{
		int status = 0;
		int childId = wait(&status);
		printf ("Child process(id %d) is returned with status: %d.\n",childId,status);
	}

	return;
}