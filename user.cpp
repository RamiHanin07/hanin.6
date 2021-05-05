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
    int resourceTable[20];
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
int msgid;
int msgidTwo;


void signalHandler(int signal);

void signalHandler(int signal){

    //Basic signal handler
    if(signal == 2)
        cout << "Interrupt Signal Received" <<endl;
    else if(signal == 20){
        cout << "Exceeded Time, Terminating Program" <<endl;
        // displayFrameTable(frameTable);
    }

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
    srand(getpid());    

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

    cout << clock->nano << " user process nano " << endl;
    cout << clock->sec << " user process sec " << endl;
    
}