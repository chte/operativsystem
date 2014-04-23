/* INSERT META COMMENT HERE */

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
