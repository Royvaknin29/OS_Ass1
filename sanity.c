#include "types.h"
#include "user.h"

int
main(int argc, char *argv[]){
    int n = 0;
    printf(1, "     Welcome to Sanity!!!\n");
    printf(1, "*************************************\n");
    if (argc < 2 ){
        printf(1, "Missing argument (n)!!\n");
            exit();
    }
    n = atoi(argv[1]);
     int i;
     int pid, realPid;
     int numOfChildren = 3 * n;
     for(i = 0; i < numOfChildren; i++){
        pid = fork();
        if (pid == 0){
            break;
        }
    }
    if (pid == 0){
        int j, k;
        realPid = getpid();
        printf(1,"I am process number: %d\n", realPid);
        if (realPid % 3 == 0){
            
            printf(1,"starting mod 0 iterations!\n");
            for(j = 0; j < 100 ; j++){
                for(k = 0; k < 1000000; k ++){
                    continue;
                }
            }
            printf(1,"Finished mod 0 iterations!\n");
        }    
        else if(realPid % 3 == 1){
        
            printf(1,"starting mod 1 iterations!\n");
            for(j = 0; j < 100 ; j++){
                for(k = 0; k < 1000000; k ++){
                    continue;
                }
            yield();
            }
            printf(1,"Finished mod 1 iterations!\n");
        }
        else if(realPid % 3 == 2){
            printf(1,"starting mod 2 iterations!\n");
            for(j = 0; j < 100 ; j++){
                sleep(1);
            }
            printf(1,"Finished mod 2 iterations!\n");
        }
    } 
    else{
        int j;
        int statsPid, retime, runtime, stime;
        for(j = 0; j < numOfChildren; j++){
            statsPid = wait2(&retime, &runtime, &stime);
            printf(1,"%s %d %s %d %s %d %s %d \n", "Process:", statsPid, "retime:", retime, "runtime:", runtime, "stime:", stime);
        }
    }

    
    return 0;
}