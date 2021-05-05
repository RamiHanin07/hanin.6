#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <csignal>
#include <bits/stdc++.h>
#include <sys/msg.h>
#include <string>
#include <time.h>
#include <bits/stdc++.h>

using namespace std;

struct simClock{
    int sec;
    int nano;
};

struct processes{
    int pid;
    int timeStartedSec;
    int timeStartedNS;
    int totalCPUTime;
    int totalTimeSystem;
    int lastBurst;
    int processPrio;
    bool typeOfSystem;
    int blockRestartSec;
    int blockRestartNS;
    bool blocked;
    bool terminated;
    int timeInReadySec;
    int timeInReadyNS;
    int availableResources[20];
};

struct mesg_buffer{
    long mesg_type;
    char mesg_text[100];
    int mesg_pid;
    int mesg_totalCPUTime;
    int mesg_totalTimeSystem;
    int mesg_lastBurst;
    int mesg_processPrio;
    int mesg_timeQuant;
    int mesg_timeUsed;
    bool mesg_terminateNow;
    bool mesg_terminated;
    bool mesg_typeOfSystem;
    bool mesg_blocked;
    int mesg_unblockNS;
    int mesg_unblockSec;
    int mesg_rqIndex;
    int mesg_ptNumber;
    int mesg_isItRead;
    int mesg_requestIndex;
    int mesg_requestResources;
    bool mesg_request;
    int mesg_releaseIndex;
    int mesg_resourceIndex;
    int mesg_releaseResources;
    bool mesg_released;
} message;

int shmidClock;
int shmidProc;
int shmidResources;
int shmidRemaining;
int msgid;
int msgidTwo;
int *resourceTable;
int *remainingTable;


void signalHandler(int signal);

void signalHandler(int signal){

    //Basic signal handler
    if(signal == 2)
        cout << "Interrupt Signal Received" <<endl;
    else if(signal == 20){
        cout << "Exceeded Time, Terminating Program" <<endl;
        // displayFrameTable(frameTable);
    }

    msgctl(msgid, IPC_RMID, NULL);
    msgctl(msgidTwo, IPC_RMID,NULL);
    shmctl(shmidClock, IPC_RMID, NULL);
    shmctl(shmidProc, IPC_RMID, NULL);
    shmctl(shmidResources, IPC_RMID, NULL);
    shmctl(shmidRemaining, IPC_RMID, NULL);
    exit(signal);
}

int main(int argc, char* argv[]){
    signal(SIGINT, signalHandler);
    signal(SIGALRM, signalHandler);
    int totalTimeTilExpire = 5;
    alarm(totalTimeTilExpire);
    struct processes *pTable;
    struct simClock *clock;
    cout << "test" << endl;
    int sizeMem = 1024;
    key_t keyClock = 6666;
    ofstream log("log.txt", ios::app);
    log.close();
    srand(getpid());
    int maxSystemTimeSpent = 15;
    int interval;
    int billion = 1000000000;
    message.mesg_pid = getpid();
    //Set UP Message Queues
    key_t messageKey = ftok("pog", 67);
    // key_t messageKeyTwo = ftok("home", 68);
    msgid = msgget(messageKey, 0666|IPC_CREAT);
    // msgidTwo = msgget(messageKeyTwo, 0666|IPC_CREAT); 


    //Shared Memory Creation for System Clock (Seconds)
    shmidClock = shmget(keyClock, sizeof(struct simClock), 0644|IPC_CREAT);
    if (shmidClock == -1) {
        perror("Shared memory");
        return 1;
    }

    clock = (simClock*)shmat(shmidClock,NULL,0);
    if(clock == (void*) -1){
        perror("Shared memory attach");
        return 1;
    }

    //Shared Memory Creation for Process Table 
    key_t keyProc = 7777;

    shmidProc = shmget(keyProc, sizeof(struct processes), 0644|IPC_CREAT);
    if (shmidProc == -1) {
        perror("Shared memory");
        return 1;
    }

    pTable = (processes*)shmat(shmidProc,NULL,0);
    if(pTable == (void*) -1){
        perror("Shared memory attach");
        return 1;
    }


    //Allocate Shared Memory of Resources
    key_t keyResources = 8888;

    shmidResources = shmget(keyResources, sizeof(resourceTable), 0644|IPC_CREAT);
    if(shmidResources == -1) {
        perror("Shared memory");
        return 1;
    }

    resourceTable = (int *)shmat(shmidResources,NULL,0);
    if(resourceTable == (void*) -1){
        perror("Shared memory attach");
        return 1;
    }

    //Allocate Shared Memory of Resources Remaining
    key_t keyRemaining = 9999;

    shmidRemaining = shmget(keyRemaining, sizeof(remainingTable), 0644|IPC_CREAT);
    if(shmidRemaining == -1) {
        perror("Shared memory");
        return 1;
    }

    remainingTable = (int *)shmat(shmidRemaining,NULL,0);
    if(remainingTable == (void*) -1){
        perror("Shared memory attach");
        return 1;
    }

    //ACTUAL CODING TIME YAY

    bool terminate = false;

    message.mesg_terminated = 0;

    int increment = 0;

    int pTableIndex = 0;

    for(int i = 0; i < 18; i++){
        if(pTable[i].pid == getpid())
            pTableIndex = i;
    }

    for(int i = 0; i < 20; i++){
        // cout << pTable[pTableIndex].availableResources[i] << endl;
        pTable[pTableIndex].availableResources[i] = 0;
    }
    
    // cout << "RESOURCE TABLE USER:" << endl;
    // for(int i = 0; i < 20; i++){
    //     cout << resourceTable[i] << " ";
    // }
    // cout << endl;

    // cout << "before while" << endl;
    while(terminate == false){
        message.mesg_type = 1;
        static int RANDREQUEST = 7;
        int doIDoAnything = rand() % RANDREQUEST + 1;
        static int OUTOFONEHUND = 100;
        int doIRequest = rand() % OUTOFONEHUND + 1;
        // cout << doIDoAnything << " ; doIDoAnything" << endl;
        // cout << doIRequest << " ; doIRequest" << endl;
        strcpy(message.mesg_text, "No Request");
        if(doIDoAnything == RANDREQUEST){
            // cout << "enter request" << endl;
            //Request if less than 50
            if(doIRequest < 50){
                // cout << "requesting resource" << endl;
                strcpy(message.mesg_text, "Request");
                message.mesg_request = true;
                log.open("log.txt", ios::app);
                log << "USER: Process: " << getpid() << " is requesting a resource at time: " << clock->sec << "s, " << clock->nano << "ns" << endl;
                log.close();
                // cout << "after log" <<endl;
                int requestMax = 20;
                int requestRand = rand() % ((requestMax - 1) + 1);
                // cout << requestRand << " requestRand" << endl;
                // cout << "before resourceTable" << endl;
                int resourceMax = resourceTable[requestRand];
                // cout << resourceTable[requestRand] << " table" << endl;
                // cout << resourceMax << " resourceMax" <<endl;
                // cout << "after resourceTable" << endl;
                if(resourceMax == 0){
                    for(int i = 0; i < 20; i++){
                        cout << resourceTable[i] << " ";
                    }
                    cout << endl;
                }
                int resourceRand = rand() % resourceMax + 1;
                // cout << "after variables" << endl;
                message.mesg_requestIndex = requestRand;
                message.mesg_requestResources = resourceRand;
                // cout << "end request" << endl;
            }
            //Release if more than 50
            else{
                cout << "releasing resource" << endl;
                strcpy(message.mesg_text, "Release");
                message.mesg_request = false;
                log.open("log.txt", ios::app);
                log << "USER: Process: " << getpid() << " is releasing a resource at time: " << clock->sec << "s, " << clock->nano << "ns" << endl;
                log.close();
                
                int processIndex;
                //Find which index in the processTable is this process
                for(int i = 0; i < 18; i++){
                    if(pTable[i].pid == getpid()){
                        processIndex = i;
                    }
                }
                //Randomly find resources it has allocated and release some of them.

                int emptyResources = 0;
                for(int i = 0; i < 20; i++){
                    if(pTable[processIndex].availableResources[i] == 0){
                        emptyResources++;
                    }
                }
                
                cout << emptyResources << " ;empty resources" << endl;
                //If there is some amount of resources available to be unallocated
                if(emptyResources != 20){
                    bool deallocated = false;
                    //Keep searching random indexes until you find one that isn't empty
                    while(deallocated == false){
                        int indexMax = 20;
                        int randIndex = rand() % ((indexMax - 1) + 1);
                        //Once you've found one that isn't empty, remove a random amount of its resources
                        if(pTable[processIndex].availableResources[randIndex] != 0){
                            int resourceMax = pTable[processIndex].availableResources[randIndex];
                            int randResources = rand() % resourceMax + 1;
                            deallocated = true;
                            message.mesg_releaseIndex = processIndex;
                            message.mesg_resourceIndex = randIndex;
                            message.mesg_releaseResources = randResources;
                            message.mesg_released = true;
                        }
                    }
                }
                else{
                    cout << "no resources currently in use" << endl;
                }
            }
            // cout << "before msgsnd" << endl;
            msgsnd(msgid, &message, sizeof(message), 0);
        }

        if(increment > 100){
            terminate = true;
        }
        increment++;
        interval = rand() % maxSystemTimeSpent + 1;
        clock->nano+= interval;
        if(clock->nano >= billion){
                clock->nano = clock->nano - billion;
                clock->sec++;
        }
    }
        //Actually terminating
        message.mesg_terminated = 1;
        msgsnd(msgid, &message, sizeof(message), 0);
}