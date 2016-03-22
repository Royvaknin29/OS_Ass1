#include "types.h"
#include "user.h"

int
main(int argc, char *argv[]){
    printf(1, "     Welcome to Testing File!!!\n");
    printf(1, "*************************************\n");

    int *retime = 0;
    int *rutime = 0;
    int *stime = 0;
    
    set_prio(3);
    printf(1, "%d%s", retime, " - retime\n");
    printf(1, "%d%s", rutime, " - rutime\n");
    printf(1, "%d%s", stime, " - stime\n");

    printf(1, "%s", "Testing finished");
    exit();
}