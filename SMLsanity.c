#include "types.h"
#include "user.h"

int
main(int argc, char *argv[]){
    int numOfFinishedProcesses = 0;
    printf(1, "     Welcome to SMLsanity File!!!\n");
    printf(1, "*************************************\n");
    int i, j, k;
    int pid;
    for(i = 0; i < 21; i++){
        pid = fork();
        if (pid == 0){
            break;
        }
    }
    if (pid == 0){ // child process
        int realPid = getpid();
        if (realPid % 3 ==  0){
            set_prio(1);
            printf(1,"Set prio 1. for proc: %d\n", realPid);
        }
        else if(realPid % 3 == 1){
            set_prio(2);
            printf(1,"Set prio 2. for proc: %d\n", realPid);

        }
        else{
            set_prio(3);
            printf(1,"Set prio 3. for proc: %d\n", realPid);
        }
        for(j = 0; j < 200 ; j++){
            for(k = 0; k < 2000000 ; k++){
                continue;
                }
            }
    } 
    else{ //papa
        int j;
        int statsPid, retime, runtime, stime;
        for(j = 0; j < 21; j++){
            statsPid = wait2(&retime, &runtime, &stime);
            numOfFinishedProcesses++;            
            printf(1,"%s %d %s %d %s %d %s %d %s %d %s\n",
             "ProcessID:", statsPid,"Priority:", (statsPid % 3) + 1,  "retime:", retime, "runtime:", runtime, "stime:", stime, "Finished Running.");

            }
        
        printf(1,"Thats it.\n");
        printf(1,"Finished %d processes..\n", numOfFinishedProcesses);
    }

    exit();
    return 0;
}