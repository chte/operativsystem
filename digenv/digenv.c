/*
 * Name:        digenv - A small program to print environment variabels
 *
 * Desciption:  The program will do the same thing as printenv | sort | $PAGER
                if no input arguments are given. If input arguments is provieded
                the program will execute as printev | grep <args> | sort | $PAGER
 *
 * Authors:     Christopher Teljstedt   (chte@kth.se)
 *              Carl Eriksson           (carerik@kth.se)
 *
 * Syntax:      ./digenv <args>
 * 
 * Example:     ./digenv
 *              ./digenv HOME
 *              ./digenv -i HOME
 * 
 */

#include "pipeutils.h" /* contains all pipeline logics */

#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    /* Get pager from environment variables if defined */
    char *pagerenv = getenv("PAGER");
    
    /*                      filter      next    sec      NULL-terminated argv       has args */
    filter_t more       = { "more",     NULL,   NULL,   (char *[]) {"more",     NULL}, 0 };
    filter_t less       = { "less",     NULL,   &more,  (char *[]) {"less",     NULL}, 0 };
    filter_t pager      = {  pagerenv,  NULL,   &less,  (char *[]) { pagerenv,  NULL}, 0 };
    filter_t sort       = { "sort",     &pager, NULL,   (char *[]) {"sort",     NULL}, 0 };
    filter_t grep       = { "grep",     &sort,  NULL,   argv                         , 1 };
    filter_t printenv   = { "printenv", &sort,  NULL,   (char *[]) {"printenv", NULL}, 0 };
    
    if (argc > 1) 
    { /* Om paramaterlista är given sätt printenvs första 
         filter i pipelinen som grep och därefter sort */
        printenv.next_filter = &grep;
    }

    /* Starta pipeline processen för argumentet */
    pipe_arg(&printenv, STDIN_FILENO);

    exit( 0 ); /* programmet avslutas normalt */
}
