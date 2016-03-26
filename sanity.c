#include "types.h"
#include "user.h"

 struct procTypeStats{
  int totalSleepTime;
  int totalReadyTime;
  int totalRuntime;
} procTypeStats;

void printTotalStats(int numOfProcs, struct procTypeStats* CPUproc, struct procTypeStats* SCPUproc,struct procTypeStats* IOproc){
        printf(1, "###########  Total Statistics Overview ###########\n\n");
        printf(1,"**** Process Type: CPU-Bound\n");
        int cpuTotalTime = CPUproc->totalRuntime + CPUproc->totalReadyTime + CPUproc->totalSleepTime;
        printf(1, "%s %d %s %d %s %d \n****\n", "AVG retime:", (CPUproc->totalReadyTime / numOfProcs), "AVG TurnAround time:", (cpuTotalTime / numOfProcs), "AVG stime:", (CPUproc->totalSleepTime / numOfProcs));
        printf(1,"**** Process Type: SCPU-Bound\n");
        int scpuTotalTime = SCPUproc->totalRuntime + SCPUproc->totalReadyTime + SCPUproc->totalSleepTime;
        printf(1, "%s %d %s %d %s %d \n****\n", "AVG retime:", (SCPUproc->totalReadyTime / numOfProcs), "AVG TurnAround time:", (scpuTotalTime / numOfProcs), "AVG stime:", (SCPUproc->totalSleepTime / numOfProcs));
        int ioTotalTime = IOproc->totalRuntime + IOproc->totalReadyTime + IOproc->totalSleepTime;
        printf(1,"**** Process Type: IO-Bound\n");
        printf(1, "%s %d %s %d %s %d \n****\n", "AVG retime:", (IOproc->totalReadyTime / numOfProcs), "AVG TurnAround time:", (ioTotalTime / numOfProcs), "AVG stime:", (IOproc->totalSleepTime / numOfProcs));

}
void updateTotalStats(struct procTypeStats* proc, int retime, int runtime, int stime){
    proc->totalSleepTime += stime;
    proc->totalReadyTime += retime;
    proc->totalRuntime += runtime;
}
int
main(int argc, char *argv[]){
    int n = 0, numOfFinishedProcesses = 0;
    struct procTypeStats CPUStats;
    struct procTypeStats SCPUStats;
    struct procTypeStats IOStats;
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
        // CPU bound process.(CPU)
        if (realPid % 3 == 0){
            for(j = 0; j < 100 ; j++){
                for(k = 0; k < 1000000; k ++){
                    continue;
                }
            }
        } 
        // short time CPU-bound(S_CPU) 
        else if(realPid % 3 == 1){
            for(j = 0; j < 100 ; j++){
                for(k = 0; k < 1000000; k ++){
                    continue;
                }
            yield();
            }
        }
        //IO bound process(IO)
        else if(realPid % 3 == 2){
            for(j = 0; j < 100 ; j++){
                sleep(1);
            }
        }
    } 
    else{//Pid != 0:
        int j, processType;;
        int statsPid, retime, runtime, stime;
        char* types [3];
        types[0] = "CPU";
        types[1] = "SCPU";
        types[2] = "IO";
        for(j = 0; j < numOfChildren; j++){
            statsPid = wait2(&retime, &runtime, &stime);
            numOfFinishedProcesses++;            
            printf(1,"%s %d %s %s %s %d %s %d %s %d %s\n", "ProcessID:", statsPid,"of type:", types[(statsPid % 3)], "retime:", retime, "runtime:", runtime, "stime:", stime, "Finished Running.");
            processType = statsPid % 3;
             switch (processType){
                case 0:  // CPU
                    updateTotalStats(&CPUStats, retime, runtime, stime);   
                break;
                case 1:  // SCPU
                    updateTotalStats(&SCPUStats, retime, runtime, stime);   
                break;
                case 2:  // IO
                    updateTotalStats(&IOStats, retime, runtime, stime); 
                break;
                default:
                     printf(1,"ERROR!\n");
            }
        }
        printTotalStats(numOfFinishedProcesses, &CPUStats, &SCPUStats, &IOStats);
        printf(1,"Thats it.\n");
        printf(1,"Finished %d processes..\n", numOfFinishedProcesses);
    }
    exit();
    return 0;
}