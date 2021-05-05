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
} message;

int shmidClock;
int shmidProc;
int shmidResources;
int msgid;
int msgidTwo;
int *resourceTable;

void signalHandler(int signal);

void displayRTable();

void signalHandler(int signal){

    //Basic signal handler
    if(signal == 2)
        cout << "Interrupt Signal Received" <<endl;
    else if(signal == 20)
        cout << "Exceeded Time, Terminating Program" <<endl;
    else if(signal == 1)
        cout << "Process Terminated" << endl;

    // msgctl(msgid, IPC_RMID, NULL);
    // msgctl(msgidTwo, IPC_RMID,NULL);
    // shmctl(shmidClock, IPC_RMID, NULL);
    // shmctl(shmidProc, IPC_RMID, NULL);
    exit(signal);
}

int main(int argc, char* argv[]){
    struct processes *pTable;
    struct simClock *clock;
    cout << "test" << endl;
    int sizeMem = 1024;
    key_t keyClock = 6666;
    ofstream log("log.txt");
    log.close();
    srand(time(NULL));

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

    key_t keyResources = 8888;

    shmidResources = shmget(keyResources, sizeof(resourceTable), 0644|IPC_CREAT);
    if(shmidResources == -1) {
        perror("Shared memory");
        return 1;
    }

    resourceTable = (int *)shmat(shmidProc,NULL,0);
    if(resourceTable == (void*) -1){
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
    }
    resourceTable[20] = -1;

    // //Output resource table after generation
    // for(int i = 0; i < 21; i++){
    //     // cout << resourceTable[i] << " ; " << i << endl;
    // }
    displayRTable();



}

void displayRTable(){

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
    log << "OSS: Remaining Resources: ";
    for(int i = 0; i < 20; i++){
        if(resourceTable[i] < 10)
            log << resourceTable[i] << "   ";
        else
            log << resourceTable[i] << "  ";
    }
    log << endl;
    log.close();
}