#include <stdlib.h>
#include <unistd.h>			/* Används av chdir som exempel */
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <stdio.h>			/* för kunna använda fget för indata från användaren */
#include <string.h>         /* definierar strings */
#include <time.h>
#include <sys/time.h>


void executeForeGround(char**,int);

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
	printf("> ");


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
    		/* Hämta andra argumentet, dvs sökvägen användaren vill byta till */
    		token = strtok(NULL, splitToken);

    		int ret;
    		/* Prova att ändra map efter användarens önskemål */
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
    	}
    	/* Användaren vill köra ett vanligt kommando */
    	else {
    		char *cmd[7]; /* = {token,NULL}; */
    		cmd[0] = token; /* Sätt första token som redan är parsat */
    		int i = 1; 

    		/* Hämta ut resten av alla tokens */
    		while(token != NULL && i < 6){
    			/* printf("%s\n", token); */
    			/* Ta ut nästa token */
    			token = strtok(NULL, splitToken);
    			/* Spar ner det som in parameter */
    			cmd[i] = token;
    			i++;
    		}

    		/* Sätt det sista parameter till NULL */
    		cmd[i] = NULL;

    		/* Titta om programmet ska köras i bakgrunden eller köras vanligt */
    		if(strcmp(cmd[i-2],"&") == 0){
    			/* Kör kommandot som background */
    			executeForeGround(cmd,0);
    		}
    		else {
    			/* Kör kommandot som foreground */
    			executeForeGround(cmd,1);
    		}
    	}
    	/* Skriv ut promt tecknet till terminalen */
    	printf("> ");
    }    
    /* Avsluta programmet med korrekt exit code */
    exit(0);
}

void executeForeGround(char **cmd,int background){
	pid_t pid;
	struct timeval start, end;

	pid = fork();
	if (pid == 0){
		execvp(cmd[0], cmd);
		printf("Unknown command\n");
	}
	else{
		int status = 0;
		printf("Spawned foreground process pid: %d\n",pid);
		gettimeofday(&start, NULL);
		int childId = wait(&status);
		gettimeofday(&end, NULL);
		printf("Foreground process %d terminated\n",childId);
		printf("wallclock time: %lf\n", (((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0));
	}

	return;
}
