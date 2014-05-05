#include <stdlib.h>
#include <unistd.h>			/* Används av chdir som exempel */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <stdio.h>			/* för kunna använda fget för indata från användaren */
#include <string.h>         /* definierar strings */


void executeForeGround(char**);

int main(int argc, char **argv) {

	char word[70];
	int count = 1;

	/* Mellanslag som används som token för strtok() */
	const char splitToken[2] = " ";

	/* Skriv ut en välkomns text som beskriver vad man kan göra */
	printf("Welcome to miniShell!\n");
	printf("    Build in commands:\n");
	printf("     - <any command supported by your OS, i.e ls>\n");
	printf("     - cd <folder path>\n");
	printf("     - exit\n");


	/* Ta emot indata från användaren */
	while (fgets(word, sizeof(word), stdin) != NULL){
        count = strlen(word); /* Längden för kommandot */
        word[--count] = '\0';  /* Ta bort new line tecknet*/

		char *token;
		/* get the first token */
   		token = strtok(word, splitToken);

        /* Exit - användaren vill sluta*/
        if(strcmp(token,"exit") == 0){
       	 	printf("Exiting...\n");
       	 	break;
    	}
    	/* cd - byta map */
    	else if(strcmp(token, "cd") == 0){
    		/* Hämta andra argumentet */
    		token = strtok(NULL, splitToken);

    		int ret;
    		ret = chdir(token);

    		/* Sökvägen användaren fanns inte */
    		if(ret != 0){
    			printf("Couldn't find directory, changing to HOME\n");
   				/* Byt till hem katalogen som defineras av HOME variablen */
    			ret = chdir(getenv("HOME"));
    			/* Fanns ingen HOME variable -> gör inget */
    			if (ret != 0){
    				printf("Couldn't find HOME variable\n");
    			} 
    			/* Byte till HOME katalogen */
    			else {
    				printf("Changed to %s\n",getenv("HOME"));
    			}
    		}
    		/* Sökvägen fanns */
    		else {
    			printf("Changed to %s\n",token);
    		}

    		/*printf("Change the directori\n"); */
    	}
    	/* Användaren vill köra ett vanligt kommando */
    	else {
    		char *cmd[] = {token,NULL};
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