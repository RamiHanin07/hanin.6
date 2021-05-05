//Rami Hanin
//5/5/2021
//Project 6: Resource Management


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

void displayRTable(processes *pTable);

void signalHandler(int signal){

    //Basic signal handler
    if(signal == 2)
        cout << "Interrupt Signal Received" <<endl;
    else if(signal == 20)
        cout << "Exceeded Time, Terminating Program" <<endl;
    else if(signal == 1)
        cout << "Process Terminated" << endl;

    msgctl(msgid, IPC_RMID, NULL);
    msgctl(msgidTwo, IPC_RMID,NULL);
    shmctl(shmidClock, IPC_RMID, NULL);
    shmctl(shmidProc, IPC_RMID, NULL);
    shmctl(shmidResources, IPC_RMID, NULL);
    shmctl(shmidRemaining, IPC_RMID, NULL);
    exit(signal);
}

int main(int argc, char* argv[]){
    struct processes *pTable;
    struct simClock *clock;
    signal(SIGINT, signalHandler);
    signal(SIGALRM, signalHandler);
    int totalTimeTilExpire = 5;
    alarm(totalTimeTilExpire);
    // cout << "test" << endl;
    int sizeMem = 1024;
    key_t keyClock = 6666;
    ofstream log("log.txt");
    log.close();
    srand(time(NULL));
    int maxTimeBetweenNewProcsNS = 500;
    int totalProcesses = 0;
    int totalTerminated = 0;
    int maxSystemTimeSpent = 15;
    int billion = 1000000000;
    int maxTimeBetweenProcesses = 500;

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

    // resourceTable[0] = 1;
    // cout << resourceTable[0];
    int MAXRESOURCES = 10;

    log.open("log.txt");
    log << "OSS: Beginning Process" << endl;
    log.close();
    int randResources = rand()% MAXRESOURCES + 1;
    for(int i = 0; i < 20; i++){
        randResources = rand()% MAXRESOURCES + 1;
        resourceTable[i] = randResources;
        remainingTable[i] = randResources;
    }

    for(int i = 0; i < 18; i++){
        pTable[i].pid = -1;
    }
    resourceTable[20] = -1;
    remainingTable[20] = -1;

    // //Output resource table after generation
    // for(int i = 0; i < 21; i++){
    //     // cout << resourceTable[i] << " ; " << i << endl;
    // }
    



    //Generate Child Processses
    char buffer[50] = "";
    char fileName[10] = "test";
    int interval = 0;
    for(int i = 0 ; i < 2; i++){
        interval = rand()% maxTimeBetweenProcesses + 1;
        clock->nano+= interval;
            if(clock->nano >= billion){
                clock->nano = clock->nano - billion;
                clock->sec++;
        }
        totalProcesses++;
        if(fork() == 0){
            pTable[i].pid = getpid();
            pTable[i].timeStartedSec = clock->sec;
            pTable[i].timeStartedNS = clock->nano;
            log.open("log.txt",ios::app);
            log << "OSS: Generating process with PID " << getpid() << " at time: " << clock->sec << " s : " << clock->nano << "ns \n";
            log.close();
            execl("./user", buffer);
        }
    }

    sleep(1);
    int increment;
    
    int displayCheck = 0;
    while(totalTerminated < totalProcesses){
        if(msgrcv(msgid, &message, sizeof(message), 1, IPC_NOWAIT) == -1){
            //If there is no message, just keep adding clock
            interval = rand() % maxSystemTimeSpent + 1;
            clock->nano+= interval;
            if(clock->nano >= billion){
                clock->nano = clock->nano - billion;
                clock->sec++;
            }
        }
        //If there is a message, do everything else
        else{
            interval = rand() % maxSystemTimeSpent + 1;
            displayCheck += interval;
            clock->nano+= interval;
            if(clock->nano >= billion){
                clock->nano = clock->nano - billion;
                clock->sec++;
            }
            if(displayCheck >= 100){
                displayRTable(pTable);
                displayCheck = 0;
            }
            //If the process terminated
            if(message.mesg_terminated == 1){
                cout << "terminating" << endl;
                log.open("log.txt", ios::app);
                log << "OSS: Terminating Process: " << message.mesg_pid << " and releasing all allocated resources" << endl;
                log.close();
                for(int i = 0; i < 18; i++){
                    if(pTable[i].pid == message.mesg_pid){
                        pTable[i].pid = -1;
                        for(int j = 0; j < 20; j++){
                            if(pTable[i].availableResources[j] != 0){
                                //Removing allocation
                                remainingTable[j] += pTable[i].availableResources[j];
                                pTable[i].availableResources[j] = 0;
                            }
                        }
                    }
                }
                totalTerminated++;
            }
            //On request resources, allocate resources
            else if(message.mesg_request == true){
                cout << "message was a request" << endl;
                cout << message.mesg_requestResources << " requestResources " << endl;
                cout << message.mesg_requestIndex << " requestIndex" << endl;
                if(remainingTable[message.mesg_requestIndex] >= message.mesg_requestResources){
                    cout << "enough resources for request" << endl;
                    remainingTable[message.mesg_requestIndex] = (remainingTable[message.mesg_requestIndex] - message.mesg_requestResources);
                    cout << remainingTable[message.mesg_requestIndex] << " ; rT remaining" << endl;
                    for(int i = 0; i < 18; i++){
                        if(pTable[i].pid == message.mesg_pid){
                            // cout << i << " ; index" << endl;
                            pTable[i].availableResources[message.mesg_requestIndex] = message.mesg_requestResources;
                        }
                    }
                }
                else{
                    cout << "not enough resources for request" << endl;
                    cout << "entering blocked queue until those resources are available" << endl;
                    //Implement Blocked Queue Thanks
                }
            }
            else if(message.mesg_request == false){
                if(message.mesg_released == true){
                    cout << "message was a release" << endl;
                    cout << message.mesg_releaseIndex << " releaseIndex" << endl;
                    cout << message.mesg_resourceIndex << " resourceIndex" << endl;
                    cout << message.mesg_releaseResources << " releaseResources" << endl;
                    // displayRTable(pTable);
                    //Releasing Specified Amount of Resources from Index
                    pTable[message.mesg_releaseIndex].availableResources[message.mesg_resourceIndex] -= message.mesg_releaseResources;
                    remainingTable[message.mesg_resourceIndex] += message.mesg_releaseResources;
                    // displayRTable(pTable);
                }
                else{
                    cout << "had no resources to deallocate" << endl;
                }

            }
        }
    }

    // cout << clock->nano << "ns" << endl;
    // cout << clock->sec << "s" << endl;
    // cout << message.mesg_text << endl;


    wait(NULL);
    displayRTable(pTable);
    msgctl(msgid, IPC_RMID, NULL);
    msgctl(msgidTwo, IPC_RMID,NULL);
    shmctl(shmidClock, IPC_RMID, NULL);
    shmctl(shmidProc, IPC_RMID, NULL);
    shmctl(shmidResources, IPC_RMID, NULL);
    shmctl(shmidRemaining, IPC_RMID, NULL);
}

void displayRTable(processes *pTable){

    //Display to Console
    for(int i = 0; i < 20; i++){
        if(i < 10){
            cout << i << "   ";
        }
        else{
            cout << i << "  ";
        }
    }
    cout << endl;
    for(int i = 0; i < 20; i++){
        if(resourceTable[i] < 10)
            cout << resourceTable[i] << "   ";
        else
            cout << resourceTable[i] << "  ";
    }
    cout << endl;


    //Log to Logfile
    ofstream log("log.txt", ios::app);
    log << endl;
    log << "OSS: Displaying Resource Table" << endl;
    log << "OSS: Resource Index:      ";
    for(int i = 0; i < 20; i++){
        if(i < 10){
            log << i << "   ";
        }
        else{
            log << i << "  ";
        }
    }
    log << endl;
    log << "OSS: Maximum Resources:   ";
    for(int i = 0; i < 20; i++){
        if(resourceTable[i] < 10)
            log << resourceTable[i] << "   ";
        else
            log << resourceTable[i] << "  ";
    }
    log << endl;
    log << "OSS: ------------------Resources In Use------------------";
    log << endl;
    int totalProcesses = 0;
    for(int i = 0; i < 18; i++){
        if(pTable[i].pid != -1){
            totalProcesses++;
        }
    }
    for(int j = 0; j < totalProcesses; j++){
        if(pTable[j].pid != -1){
            log << "OSS: Process: " << pTable[j].pid << ":      ";
            for(int i = 0; i < 20; i++){
                if(pTable[j].availableResources[i] < 10)
                    log << pTable[j].availableResources[i] << "   ";
                else 
                    log << pTable[j].availableResources[i] << "  ";
            }
            log << endl;
        }
    }
    for(int i = 0 ; i < 20; i++){
        log << "------";
    }
    log << endl;
    log << "OSS: Resources Remaining: ";
    for(int i = 0; i < 20; i++){
        if(remainingTable[i] < 10)
            log << remainingTable[i] << "   ";
        else 
            log << remainingTable[i] << "  ";
    }
    log << endl;
    log.close();
}