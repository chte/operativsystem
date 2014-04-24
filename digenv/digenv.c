/* INSERT META COMMENT HERE */

/*
 * Name:        digenv - A small program to print envoriement variabels
 *
 * Desciption:  The program will do the same thing as printenv | sort | $PAGER
                if no input arguements is pased by. If inputarguments is provieded
                the program will execute as printev | grep <args> | sort | $PAGER
 *
 * Authors:     Christopher Teljstedt   (chte@kth.se)
 *              Carl Eriksson           (carerik@kth.se)
 *
 * Syntax:      digenv <args>
 * 
 * Example:     digenv HOME
 * 
 */

#include "pipeutils.h" /* contain all pipeline logics */

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    char *pagerenv = getenv("PAGER");
    
    /*                      filter      next    sec      NULL-terminated argv          */
    filter_t more       = { "more",     NULL,   NULL,   (char *[]) {"more",     NULL} };
    filter_t less       = { "less",     NULL,   &more,  (char *[]) {"less",     NULL} };
    filter_t pager      = {  pagerenv,  NULL,   &less,  (char *[]) { pagerenv,  NULL} };
    filter_t sort       = { "sort",     &pager, NULL,   (char *[]) {"sort",     NULL} };
    filter_t grep       = { "grep",     &sort,  NULL,   argv,                         };
    filter_t printenv   = { "printenv", &sort,  NULL,   (char *[]) {"printenv", NULL} };
    

    if (argc > 1) 
    { /* Om paramaterlista är given sätt printenvs första 
         filter i pipelinen som grep och därefter sort */
        printenv.next_filter = &grep;
    }

    /* Starta pipeline processen för argumentet */
    pipe_arg(&printenv, STDIN_FILENO);

    exit( 0 ); /* programmet avslutas normalt */
}
