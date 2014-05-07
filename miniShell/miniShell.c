/*
 * Name:			miniShell - A small shell that can execute programs in
 *								foreground and background and change folder.
 *
 * Description:		The shell is a small shell that gives the possibility to
 *					change the folder by using chdir, if the requested folder
 *					doesn't exist the program will change to the HOME folder.
 *					The shell can also start programs in foreground, which 
 *					will print out the executing time, when exited. Programs
 *					can also be runned in background mode by adding & after
 *					the program name.
 *
 * Authors:			Carl Eriksson			(carerik@kth.se)
 *					Christopher Teljstedt   (chte@kth.se)
 *
 * Syntax:			./miniShell
 *
 * Commands:		cd <foler path>  - change the current folder
 *					<program name>	 - runs the program in foreground
 *					<program name> & - runs the program in background
 *
 */

#include <stdlib.h>			/* definierar bland annat exit() */
#include <unistd.h>			/* Används av chdir som exempel */
#include <stdio.h>			/* för input och output från och till användaren */
#include <sys/types.h>		/* definierar bland annat typen pid_t */
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <stdio.h>			/* för kunna använda fget för indata från användaren */
#include <string.h>         /* definierar strings */
#include <time.h>			/* Används för beräkning av körtiden */
#include <sys/time.h>		/* Används för beräkning av körtiden */
#include <signal.h>			/* För att hantera Ctrl+C */

int status; /* för returvärden från child-processer */
static const char *WS = " \t\n"; /* End of line tecken för att upptäcka tom input från användaren */

/**
 * executeProgram() - Kör ett kommando
 *
 * Funktionen kör execvp() med de specifierade kommandon
 * som finns som indata
 *
 * In:	char** cmd, 	de kommandon och inparams som ska köras
 * 		int background,	om den ska köras som background eller foreground  
 *
 */
void executeProgram(char **cmd,int background);

/**
 * checkStatus() - Kontrollerar statusen efter ett kört kommando
 * 
 * Skriver ut information till användaren efter ett kommando har kört klart
 *
 * In:	int status, 	Status koden från waitdpid()
 *		int background,	Ifall processen kördes som en background eller foreground
 *		int childId,	Id för child processen som kördes
 *
 */
void checkStatus(int status,int background,int childId);

/**
 * hold() - Körs i main loopen, funktionen
 *			kontrollerar om har skett någon 
 *			ändring i någon bakgrunds program	
 *
 */
int hold();

/**
 * INThandler() - används för att aktivera Ctrl+C
 *				  i child processer.
 *
 */
void INThandler();

/* Programmet startar här */
int main(int argc, char **argv) {
	/* Fångar upp Ctrl+c signalen och ignorerar den */
	signal(SIGINT, SIG_IGN);

	/* Används för input från användaren, 70 tecken är max input enligt specification */
	char word[70]; 

	/* Längden för input */
	int count = 1;

	/* Mellanslag som används som token för strtok() */
	const char splitToken[2] = " ";

	/* Skriv ut en välkomns text som beskriver vad man kan göra */
	printf("Welcome to miniShell!\n");
	printf("    Commands:\n");
	printf("     - <any command supported by your OS, i.e ls>\n");
	printf("     - cd <folder path>\n");
	printf("     - exit\n");
	printf("> ");

	/* Ta emot indata från användaren */
	while (fgets(word, sizeof(word), stdin) != NULL){
		/* Titta om några bakgrunds processer har gjort någon ändring */
		while(hold()){}

		/* Titta så att användaren inte skicka in tom sträng */
		if(strspn(word, WS) == strlen(word)){
			printf("> ");
			continue; /* Forsätt, eftersom vi inte kan göra något med tomt ḱommando */
		}   

        count = strlen(word); /* Längden för kommandot */
        word[--count] = '\0';  /* Ta bort new line tecknet*/

		char *token;
		/* get the first token */
   		token = strtok(word, splitToken);

        /* Exit - användaren vill sluta*/
        if(strcmp(token,"exit") == 0){
       	 	printf("Exiting...\n"); /* Säg hejdå till användaren */
       	 	break; /* Avsluta main loopen så programmes avslutas */
    	}
    	/* cd - byta map */
    	else if(strcmp(token, "cd") == 0){
    		/* Hämta andra argumentet, dvs sökvägen användaren vill byta till */
    		token = strtok(NULL, splitToken);

    		/* Retur status för map byte */
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
    		char *cmd[6]; /* Plats max 5 kommandon samt NULL som sista argument */
    		cmd[0] = token; /* Sätt första token som redan är parsat */
    		int i = 1; /* Räknare för kommandon */

    		/* Hämta ut resten av alla tokens */
    		while(token != NULL && i < 6){
    			/* Ta ut nästa token */
    			token = strtok(NULL, splitToken);
    			/* Spar ner det som in parameter */
    			cmd[i] = token;
    			/* Öka räknaren för antal kommandon*/
    			i++;
    		}
    		/* Titta om programmet ska köras i bakgrunden eller köras vanligt */
    		if(strcmp(cmd[i-2],"&") == 0){
    			/* Sätt det sista parameter till NULL */
    			cmd[i-2] = NULL;
    			/* Kör kommandot som background */
    			executeProgram(cmd,1);
    		}
    		else {
    			/* Sätt det sista parameter till NULL */
    			cmd[i-1] = NULL;
    			/* Kör kommandot som foreground */
    			executeProgram(cmd,0);
    		}
    	}
    	/* Titta om några bakgrunds processer har gjort någon ändring */
		while(hold()){}

   		/* Skriv ut prompten efter vi är klara med allt */
		printf("> ");
    }    
    /* Avsluta programmet med korrekt exit code */
    exit(0);
}

int hold(){
	/* Titta om det har hänt en förändring i någon av child processer */ 
	int pid = waitpid(-1, &status, WNOHANG);

    /* En ändring i en childprocess */
    if(pid > 0 ){
    	/* Skriv  till användaren vad som har ändrats */
    	checkStatus(status, 1, pid);
    }else {
    	/* Skicka tillbaka att det inte finns några mer child processer att undersöka */
    	return 0;
    }
    /* Säg att det kanske kan finnas fler child processer att undersöka */
    return pid + 1;
}


void executeProgram(char **cmd,int background){
	/* Process id */
	pid_t pid;
	/* Start och stop för tidtagning */
	struct timeval start, end;
	/* Skapa en ny adressrymd för barnet */
	pid = fork();
	/* Barnets kod */
	if (pid == 0){
		/* Sätt att Ctrl+C fungerar att göra i barn processen */
		signal(SIGINT, INThandler);
		/* Kör kommandot som kom som inparamet */
		execvp(cmd[0], cmd);
		/* Ifall det inte finns något sådant kommando skriv ut det till användaren */
		printf("Unknown command\n");
	}
	/* Parent processen */
	else{
		/* Process id för barnet */
		int childId = 0;
		/* Titta om det är en bakgrund process som körs eller inte */
		if(background == 0){
			/* Skriv ut att barn processen har startat */
			printf("Spawned foreground process pid: %d\n",pid);
			/* Starta tidtagningen */
			gettimeofday(&start, NULL);
			/* Vänta tills barnet har kört klart */
			childId = waitpid(pid, &status, 0);
			/* Avsluta tidtagningen */
			gettimeofday(&end, NULL);
			/* Skriv ut statusen för den terminerade child processen */
			checkStatus(status,background,childId);
			/* Räkna ut hur lång tid det tog att köra, skriv ut det sen */
			printf("wallclock time: %lf\n", (((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000000.0));
		}
		/* Användaren vill starta en bakgrundsprocess
		 * -> parent programmet ska inte vänta på att barnet kör klart
		 */
		else{
			/* Skriv ut vad som händer till användaren */
			printf("Spawned background process pid: %d\n",pid);
			/* Med WNOHANG specificerat kommer vi inte vänta på att barnet kör klart
			 * utan koden kommer bara att fortsäta förbi.
			 * Statusen för barn processen kontrolleras istället i funktionen hold()
			 */
			childId = waitpid(-1, &status, WNOHANG);
		}	
	}
	return;
}

void INThandler(){
	/* Stäng programmet som körs */
	_exit(2);
}

void checkStatus(int status,int background,int childId){
	/* Programmet avslutade via exit*/ 
	if (WIFEXITED(status)){
		/* Olika outputs för bakgrund och förgrunds processer */
		if(background == 0){
			printf("Foreground process %d terminated\n",childId);
		}
		else{
			printf("Background process %d terminated\n",childId);
		}
	} 
	/* Programmet avslutates via en signal, tex Ctrl+C */
	else if (WIFSIGNALED(status)) {
		/* Olika outputs för bakgrund och förgrunds processer */
		if(background == 0){
			printf("Foreground process %d terminated\n",childId);
		}
		else{
			printf("Background process %d terminated\n",childId);
		}
	}
	/* Programmet stoppades via en signal*/
	else if(WIFSTOPPED(status)){
		printf("stopped by signal %d\n", WSTOPSIG(status));
	}
	/* Programmet fortsatte köras */
	else if(WIFCONTINUED(status)){
		printf("continued\n");
	}
}
