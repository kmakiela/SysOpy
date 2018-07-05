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
#include <errno.h>
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
char* buffer;
int sendMsgqID;
struct timespec tp;
int shaved =0;
struct sembuf recMsgSem;
struct sembuf sendMsgSem;
int semID;

void catchSIGINT(int signo){
    printf(RED"\nGunned down by SIGINT\n"DEFAULT);
    free(buffer);
    exit(0);
}

void catchSIGUSR1(int signo, siginfo_t *info, void *ucontext){

    //sat in seat
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\t"CYAN"%d"GREEN"\tSat down\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, getpid());
    //kill(info->si_pid, SIGUSR1);
    sendMsgSem.sem_op=1;
    semop(semID, &sendMsgSem, 1);

    //waiting for shave to complete
    recMsgSem.sem_op =-1;
    semop(semID, &recMsgSem, 1);


    //leave
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\t"CYAN"%d"BLUE"\tLeaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec,getpid());
    sendMsgSem.sem_op=1;
    semop(semID, &sendMsgSem, 1);

    shaved = 1;
}

void catchSIGUSR2(int signo){
    printf(CYAN"%d"RED"\tReceived SIGUSR2\n"DEFAULT, getpid());
}

int main(int argc, char* argv[]){

    if (argc!=3){
        printf(RED"Wrong arguments, use ./client N M\n"DEFAULT);
        return 1;
    }

    int noOfClients = stringToInt(argv[1]);
    int noOfShaves = stringToInt(argv[2]);

    struct sigaction act;
    act.sa_handler = catchSIGINT;
    sigaction(SIGINT, &act, NULL);

    act.sa_sigaction = catchSIGUSR1;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = catchSIGUSR2;
    sigaction(SIGUSR2, &act, NULL);


    //opening public queue
    const char* homePath;
    if((homePath = getenv("HOME")) == NULL){
        printf(RED"Can't get home variable.\n"DEFAULT);
    }
    key_t parentKey = ftok(homePath, PROJECT_NO);
    sendMsgqID = msgget(parentKey, 0);

    //opening semaphore
    semID = semget(parentKey, 0, 0);
    struct sembuf sem;
    sem.sem_num = 0;
    sem.sem_flg = 0;

    recMsgSem.sem_num = 2;
    recMsgSem.sem_flg = 0;

    sendMsgSem.sem_num = 1;
    sendMsgSem.sem_flg = 0;

    //opening shared memory
    int shmID = shmget(parentKey, 3*sizeof(semID)+sizeof(getpid()), 0);//uwaga czy 2 arg nie ma byc 0!!!
    int* shmAddr = (int *) shmat(shmID, NULL, 0);

    size_t bufSize = MAX_MSG_SIZE;
    buffer = (char*) malloc(bufSize*sizeof(char));

    pid_t childPID;
    int i=0;
    for(i; i<noOfClients; i++){

       childPID = fork();
       if(childPID==0){
            while(noOfShaves>0){
                //lock FIFO and shm
                sem.sem_op=-1;
                semop(semID, &sem, 1);

                //check if barber is awake
                if(shmAddr[0]){
                    //awake
                    if(shmAddr[2]<shmAddr[1]){
                        //free seats
                        clock_gettime(CLOCK_MONOTONIC, &tp);
                        printf("[%d:%ld]\t"CYAN"%d\t"DARKRED"Waiting in queue.\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec,getpid());

                        //more people in queue
                        shmAddr[2]++;

                        //sitting in the queue
                        struct myMsgBuf mMsg;
                        mMsg.pid = getpid();
                        mMsg.mType = 1;
                        strcpy(mMsg.msg, " ");
                        mMsg.msg[strlen(mMsg.msg)-1]='\0';
                        if((i=msgsnd(sendMsgqID, (void*) &mMsg, sizeof(getpid())+strlen(mMsg.msg)+1, 0))<0){
                            clock_gettime(CLOCK_MONOTONIC, &tp);
                            printf(RED"Size: %ld\n"DEFAULT, sizeof(getpid())+strlen(mMsg.msg)+1);
                                    perror(RED"msgsnd err:"DEFAULT);
                            printf(RED"%d\tCant send to %d! msgsnd:%d , %d.%ld\n"DEFAULT,getpid(), sendMsgqID, i, (int) tp.tv_sec, tp.tv_nsec);
                            sem.sem_op=1;
                            semop(semID, &sem, 1);
                            return 1;
                        }
                        //unlocking FIFO and shm
                        sem.sem_op=1;
                        semop(semID, &sem, 1);

                        //receive confirmation - I am invited SIGUSR1
                        //sigwait(&sigmask, &sig);
                        while(!shaved){
                            ;
                        }
                        noOfShaves--;
                        shaved=0;
                        printf(CYAN"\t\t\t%d"DEFAULT"\t%d more shaves\n", getpid(), noOfShaves);

                    }else{
                        //no free seats
                        clock_gettime(CLOCK_MONOTONIC, &tp);
                        printf("[%d:%ld]\t"CYAN"%d"DARKRED"\tNo seats available\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec,getpid());
                        //open FIFO nad shm
                        sem.sem_op=1;
                        semop(semID, &sem, 1);
                    }
                }else{
                    //asleep
                    clock_gettime(CLOCK_MONOTONIC, &tp);
                    printf("[%d:%ld]\t"CYAN"%d"DEFAULT"\tWaking the barber up\n", (int) tp.tv_sec, tp.tv_nsec,getpid());

                    //wake up
                    kill(shmAddr[3], SIGUSR2);

                    //wait for conformation
                    recMsgSem.sem_op = -1;
                    semop(semID, &recMsgSem, 1);

                    //unlocking FIFO and shm
                    sem.sem_op=1;
                    semop(semID, &sem, 1);

                    clock_gettime(CLOCK_MONOTONIC, &tp);
                    printf("[%d:%ld]\t"CYAN"%d"GREEN"\tSat down\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, getpid());

                    //letting know that i'm in a chair
                    sendMsgSem.sem_op = 1;
                    semop(semID, &sendMsgSem, 1);

                    //waiting for him to finish
                    recMsgSem.sem_op = -1;
                    semop(semID, &recMsgSem, 1);

                    clock_gettime(CLOCK_MONOTONIC, &tp);
                    printf("[%d:%ld]\t"CYAN"%d"BLUE"\tLeaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec,getpid());

                    //let the barber know i left
                    sendMsgSem.sem_op = 1;
                    semop(semID, &sendMsgSem, 1);
                    noOfShaves--;

                    printf(CYAN"\t\t\t%d"DEFAULT"\t%d more shaves\n", getpid(), noOfShaves);

                }

            }


           free(buffer);
           //unlocking shm
           shmdt(shmAddr);
           printf(CYAN"\t\t\t%d\t"YELLOW"I'm finished\n"DEFAULT, getpid());
           return 0;
       }


    }

    while (noOfClients){
        wait(NULL);
        noOfClients--;
        }
    free(buffer);
    //unlocking shm
    shmdt(shmAddr);
    printf(CYAN"%d\t"DEFAULT"I spawned what I had to spawn - I'm out\n", getpid());
    return 0;
}
