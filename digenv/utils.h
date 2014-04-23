#ifndef PIPE_H
#define PIPE_H

/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> 		/* definierar bland annat typen pid_t */
#include <sys/wait.h> 		/* definierar bland annat WIFEXITED */
#include <errno.h> 			/* definierar felkontrollvariabeln errno */
#include <stdio.h> 			/* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> 		/* definierar bland annat exit() */
#include <unistd.h> 		/* definierar bland annat fork() */
#include <stdbool.h>		/* definierar primitiva typen bool */
#include <string.h>			/* definierar strings */

/**
 * En struktur för pipeline filter.
 *
 * Strukturen innehåller information om nuvarande filter som ska köras
 * samt en pekare till nästa filter i pipeline flödet. Om filtret inte 
 * finns kan en 
 */
typedef struct filter {
    const char *;          		/* File to execute. */
    char * const *argv;         /* NULL-terminated array of arguments. */
    struct filter *next;     	/* Pekare till nästa pipeline filter. */
    struct filter *secondary; 	/* Pekare till ett andrahandsval om inte filtret finns */
    int err;                    /* Error from last execution attempt. */
} filter;

void pipe(filter *f, int in);


/**
 * Invokes a command.
 *
 * This function will try to run the given command using execvp(). If it fails,
 * the fallback command is tried instead. If all fallbacks fails, the function
 * prints an error to stderr and exits the process with EXIT_FAILURE.
 *
 * Commands with a NULL file field are ignored.
 */
void run(filter *f) {
    /* Start with the given command, then successively try each fallback. */
    filter *curr_f = f;
    while (curr_f) {
        if (curr_f->file) {
            execvp(curr_f->file, curr_f->argv);
            curr_f->err = errno;
        }
        curr_f = curr_f->fallback;
    }

    /* All fallbacks failed. Print error and exit. */
    fprintf(stderr, "No filter found:\n");
    curr_f = f;
    while (curr_f) {
        if (curr_f->file) {
            fprintf(stderr, "  %s: %s\n", curr_f->file, strerror(curr_f->err));
        }
        curr_f = curr_f->fallback;
    }
    exit(EXIT_FAILURE);
}

/**
 * Turns the file descriptor new into a copy of old.
 *
 * If the redirection succeeds, old will be closed. If old and new is the same file
 * descriptor, this function is a no-op. If redirection fails, the function prints
 * an error and exits the process with EXIT_FAILURE.
 */
void dup_pfd(int old, int new) {
    if (new != old) {
        if (dup2(old, new) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
        } else if (close(old)) {
            perror("close");
            _exit(EXIT_FAILURE);
        }
    }
}


/**
 * Runs a pipeline starting with the given command and input file descriptor.
 *
 * This function will construct a new pipe for communication between the given
 * command and the next, fork a new child process in which the command is executed,
 * then call itself recursively to set up the rest of the pipeline. Input for
 * the pipeline is taken from the given file descriptor. If at some point an error
 * is encountered, an error message is printed and the recursion stops.
 */
void pipe_arg(filter *f, int in) {
    int pfd[2];
    pid_t pid;
    int status;

    if (f->next == NULL) {
        /* Last command, stop forking. */
        dup_pfd(in, STDIN_FILENO);
        run(command);
        return;
    } 


    /* Create pipe. */
    if (pipe(pfd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    /* Fork a child process. */
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        /* Running in child. */
        if (close(pfd[0])) {
            perror("close");
            _exit(EXIT_FAILURE);
        }
        dup_pfd(in, 	STDIN_FILENO);
        dup_pfd(pfd[1], STDOUT_FILENO);
        run(command);
    } else {
        /* Running in parent. */
        if (close(pfd[1])) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        if (close(in)) {
            perror("close");
            exit(EXIT_FAILURE);
        }
        wait(&status);
        if (!WEXITSTATUS(status)) {
            /* Child exited normally, run rest of pipeline */
            pipe_arg(command->next, fd[0]);
        }
    }
}