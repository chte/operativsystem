#ifndef PIPEUTILS_H
#define PIPEUTILS_H

#include <sys/types.h>      /* definierar bland annat typen pid_t */
#include <sys/wait.h>       /* definierar bland annat WIFEXITED */
#include <errno.h>          /* definierar felkontrollvariabeln errno */
#include <stdio.h>          /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h>         /* definierar bland annat exit() */
#include <unistd.h>         /* definierar bland annat fork() */
#include <stdbool.h>        /* definierar primitiva typen bool */
#include <string.h>         /* definierar strings */

#define READ 0
#define WRITE 1

pid_t pid;  /* för child-processens PID vid fork() */
int status; /* för returvärden från child-processer */

/**
 * En struktur för filter.
 *
 * Strukturen innehåller information om nuvarande filter som ska köras
 * samt en pekare till nästa filter i pipeline-flödet. Om filtret inte 
 * finns kan den hålla en pekare till ett sekundärt alternativ.
 */
typedef struct filter_s {
    const char * file;              /* Filter att exekevera */
    struct filter_s *next_filter;   /* Pekare till nästa pipeline filter */
    struct filter_s *secondary;     /* Pekare till ett andrahandsval om inte filtret finns */
    char * const *argv;             /* array av argument som avslutas med NULL-terminator */
    int arg;                        /* cmd argument */
} filter_t;

/**
 * Kör ett filter
 *
 * Funktionen kör execvp() på givna filtret. Om filtret inte finns
 * körs dess sekundära alternativ osv. Filter som är null hoppas över.
 *
 * In: filter_t
 * 
 */
void run(filter_t *curr_filter);

/**
 * dupe() ersätter std-in/out med duplicerad läs/skriv-ände på pipen.
 * Om den gamla och nya gamla läs/skriv-änden är samma ignoreras 
 * denna funktion.
 */
void dupe(int old_fileno, int new_fileno);

/**
 * hold() körs i parent-processen och väntar
 * på att child-processen ska köras klart innan
 * parent-processen kör resterande kod.
 */
void hold(int pid, filter_t *f);

/**
 * pipe_arg() skapar en pipeline mellan flera filter som specificeras
 * delvis i runtime som startargument och hårdkodat.
 *
 * Funktionen anropas rekursivt för varje filter och next_filter och skapar 
 * en pipe med given filbeskrivare. If at some point an error
 * is encountered, an error message is printed and the recursion stops.
 */
void pipe_arg(filter_t *f, int in);


/* Härefter finner du koden :) */

void run(filter_t *curr_filter) {
    filter_t *filter = curr_filter;
    while (filter) { /* kör execvp() på valt filter, om fel inträffas kör dess alternativa filter */
        if (filter->file) {
            execvp(filter->file, filter->argv);
            /* om vi når denna rad har execvp misslyckats */
        }
        /* ett secondary filter testas om det finns */
        filter = filter->secondary; 
    }
    
    { /* Alla säkundära filter misslyckades. */
        fprintf(stderr, "execvp() failed ");
        _exit( 1 ); /* exit() ej pålitlig i child-processer så _exit() måste användas */
    }
}

void dupe(int old_fileno, int new_fileno){
    if(old_fileno == new_fileno)
    { /* för första nivån av rekursionen vill vi inte kopiera  från stdin till stdin */
        return;       
    } 

    if (dup2(old_fileno, new_fileno) == -1)   
    { /* ersätt std-in/out med duplicerad läs/skriv-ände på pipen */
         perror( "dup2() failed" );
         _exit( 1 ); /* exit() ej pålitlig i child-processer så _exit() måste användas */ 
    }
}

void hold(int pid, filter_t *f){
    wait(&status);
    if( WIFEXITED( status ) ) 
    { /* child-processen har kört klart */
        if( !WEXITSTATUS( status ) ) 
        {  /* child-processen kördes klart utan problem */
            return; /* återvänd som no-op */
        } else if( !f->arg ) 
        /* kolla så arg inte är satt till 1, 
           isåfall kör grep med en eller flera 
           parametrar */
        { /* fel inträffade i child-processen */
            fprintf( stderr, "Child (pid %ld) failed with exit code %d\n", (long int) pid, WEXITSTATUS( status ) );
            exit( 1 ); /* exit() ej pålitlig i child-processer så _exit() måste användas */
        }
    } 
    else 
    {
        if( WIFSIGNALED( status ) ) 
        {  /* child-processen avbröts av signal */
             fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n", (long int) pid, WTERMSIG( status ) );
             _exit( 1 ); /* exit() ej pålitlig i child-processer så _exit() måste användas */
        }
    }
}

void pipe_arg(filter_t *filter, int in) {
    int pfd[2];

    if (filter->next_filter == NULL) 
    { /* basfall uppfyllt, det finns inget next_filter */ 
        dupe(in, STDIN_FILENO);  /* kopiera förgående filbeskrivares läsände till stdin */
        run(filter);             /* kör filter med execvp() */

        { /* stäng förgående filbeskrivares läsände innan avslut */
            if (close(in)) { fprintf( stderr, "close() failed" ); exit( 1 ); }
        }
        return;
    } 

    {  /* skapa en pipe */
        if (pipe(pfd) == -1) /* avsluta programmet om pipe() misslyckas */
            { perror( "pipe() failed" ); exit( 1 ); }
    }

    pid = fork(); /* skapa child-processen */
    /** 
     * fork() skapar en separat adressrymd för barnet.
     * Barnet har en exakt kopia av alla minnessegment
     * som föräldern har. Båda exekeveras parallelt och
     * därför behöver föräldern vänta tills barnet är 
     * klart med hjälp av funktionen hold().
     */
    if (pid == 0) { /* kod som körs i child-processen */
        if (close(pfd[READ])) /* stäng nuvarande filbeskrivares läsände */
            { fprintf( stderr, "close() failed" ); _exit( 1 ); }

        dupe(pfd[WRITE], STDOUT_FILENO); /* kopiera nuvarande filbeskrivares skrivände till stdout */
        dupe(in,         STDIN_FILENO);  /* kopiera förgående filbeskrivares läsände till stdin */

        if (close(in)) /* stäng förgående filbeskrivares läsände */
            { fprintf( stderr, "close() failed" ); _exit( 1 ); }
 
        run(filter);  /* kör filter med execvp() */

    } else { /* kod som körs i parent-processen */
        if (pid == -1) /* misslyckades att skapa en child-process */
            { perror( "fork() failed" ); exit( 1 ); }

        hold(pid,filter);

        if (close(pfd[WRITE])) /* stäng nuvarande filbeskrivares skrivände */
            { fprintf( stderr, "close() failed" ); exit( 1 ); }

        /* rekurera till nästkommande filter i pipelinen och skicka med 
           nuvarande filbeskrivarens läsände till nästa child-process */ 
        pipe_arg(filter->next_filter, pfd[READ]);
    } 
}

#endif /* PIPEUTILS_H */
