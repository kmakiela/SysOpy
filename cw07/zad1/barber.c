#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/resource.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "ipcutil.h"





int stringToInt(char* s){
    int result = 0;
    for(int i=0; i<strlen(s); i++){
        result*=10;
        result+=(s[i]-'0');
    }
    return result;
}

//resources
int msqID;
int semID;
int* shmAddr;
int shmID;
struct timespec tp;
pid_t myPID;
struct sembuf recMsgSem;
struct sembuf sendMsgSem;

void closeRes(void){
    //closing queue
    msgctl(msqID, IPC_RMID, NULL);
    //closing semaphore
    semctl(semID, 1, IPC_RMID);
    //closing shm
    shmdt(shmAddr);
    shmctl(shmID, IPC_RMID, NULL);
}

void catchSIGINT(int signo){
    printf(RED"\nGunned down by SIGINT\n"DEFAULT);
    SERVER_STOP=1;
    closeRes();
    exit(0);
}

void catchSIGTERM(int signo){
    printf(RED"\nGunned down by SIGTERM\n"DEFAULT);
    SERVER_STOP=1;
    closeRes();
    exit(0);
}

void catchSIGUSR1(int signo){
    printf(RED"\nReceived SIGUSR1\n"DEFAULT);
}

void catchSIGUSR2(int signo, siginfo_t *info, void *ucontext){
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]"YELLOW"\tI am awake now\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec);

    //flag - awake
    shmAddr[0] = 1;

    //send confirmation
    sendMsgSem.sem_op = 1;
    semop(semID, &sendMsgSem, 1);

    //waiting for him to take a seat
    recMsgSem.sem_op=-1;
    semop(semID, &recMsgSem, 1);


    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\tClient "CYAN"%d"GREEN" is being shaved\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, info->si_pid);

    //letting client know shaving is complete
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\tClient "CYAN"%d"DARKRED" finished shaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, info->si_pid);

    sendMsgSem.sem_op = 1;
    semop(semID, &sendMsgSem, 1);

    recMsgSem.sem_op=-1;
    semop(semID, &recMsgSem, 1);
}

int main(int argc, char* argv[]){

    if (argc!=2){
        printf(RED"Wrong arguments, use ./barber N\n"DEFAULT);
        return 1;
    }

    myPID = getpid();

    struct sigaction act;
    act.sa_handler = catchSIGINT;
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = catchSIGTERM;
    sigaction(SIGTERM, &act, NULL);

    act.sa_handler = catchSIGUSR1;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_sigaction = catchSIGUSR2;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR2, &act, NULL);


    //creating public queue
    const char* homePath;
    if((homePath = getenv("HOME")) == NULL){
        printf(RED"Can't get home variable.\n"DEFAULT);
    }
    key_t myKey = ftok(homePath, PROJECT_NO);
    msqID = msgget(myKey, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    //creating semaphore
    semID = semget(myKey, 3, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    union semun arg;
    arg.val = 1;    //sem open
    semctl(semID, 0, SETVAL, arg);

    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;

    arg.val = 0;    //sem closed
    semctl(semID, 1, SETVAL, arg);

    //in clients nums are reversed
    recMsgSem.sem_num = 1;
    recMsgSem.sem_flg = 0;

    arg.val = 0;    //sem closed
    semctl(semID, 1, SETVAL, arg);

    sendMsgSem.sem_num = 2;
    sendMsgSem.sem_flg = 0;

    //creating shared memory
    shmID = shmget(myKey, 3*sizeof(semID)+sizeof(myPID), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    shmAddr = (int *) shmat(shmID, NULL, 0);

    shmAddr[0] = 1; //awake
    shmAddr[1] = stringToInt(argv[1]);  //queusize
    shmAddr[2] = 0; //clients in queue
    shmAddr[3] = myPID;

    struct myMsgBuf recMsg;

    while(1){
        //lock FIFO and shm
        sem.sem_op=-1;
        semop(semID, &sem, 1);

        //check if anyone is waiting
        int k = (int) msgrcv(msqID, &recMsg, MAX_MSG_SIZE, 0, IPC_NOWAIT);
        if(k>0) {
            int sig;

            //one less client in queue
            shmAddr[2]--;

            //invite client for shaving
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"DEFAULT" invited\n", (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);
            kill(recMsg.pid, SIGUSR1);

            //receive confirmation - client is sitting in chair SIGUSR1
            recMsgSem.sem_op = -1;
            semop(semID, &recMsgSem, 1);


            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"GREEN" is being shaved\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);

            //unlocking FIFO and shm
            sem.sem_op=1;
            semop(semID, &sem, 1);

            //shaving itself:

            //letting client know shaving is complete
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"DARKRED" finished shaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);
            sendMsgSem.sem_op = 1;
            semop(semID, &sendMsgSem, 1);

            recMsgSem.sem_op = -1;
            semop(semID, &recMsgSem, 1);

        }
        else{
            //go to sleep - no one in the queue
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\t"MAGENTA"Going to sleep\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec);

            //flag - asleep
            shmAddr[0] = 0;

            //open FIFO nad shm
            sem.sem_op=1;
            semop(semID, &sem, 1);

            //don't do anything while asleep!
            while(!shmAddr[0]){
                ;
            }

        }

    }

    printf("Server stopped.\n");
    return 0;
}
