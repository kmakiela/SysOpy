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
#include <mqueue.h>
#include <semaphore.h>
#include <sys/mman.h>
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
mqd_t msqID;
sem_t* semID;
int* shmAddr;
struct timespec tp;
pid_t myPID;
sem_t* recMsgSem;
sem_t* sendMsgSem;



void closeRes(void){
    //closing queue
    mq_close(msqID);
    mq_unlink("/queue");
    //closing semaphore
    sem_close(semID);
    sem_close(recMsgSem);
    sem_close(sendMsgSem);
    sem_unlink("/sem");
    sem_unlink("/recSem");
    sem_unlink("/sendSem");
    //closing shm
    munmap(shmAddr, 3*sizeof(semID)+sizeof(myPID));
    shm_unlink("/mem");
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

    //send conformation
    sem_post(sendMsgSem);

    //waiting for him to take a seat
    sem_wait(recMsgSem);

    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\tClient "CYAN"%d"GREEN" is being shaved\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, info->si_pid);

    //letting client know shaving is complete
    clock_gettime(CLOCK_MONOTONIC, &tp);
    printf("[%d:%ld]\tClient "CYAN"%d"DARKRED" finished shaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, info->si_pid);


    sem_post(sendMsgSem);

    //waiting for client to leave
    sem_wait(recMsgSem);

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
    struct mq_attr attr;
    attr.mq_maxmsg = stringToInt(argv[1]);
    attr.mq_msgsize = sizeof(struct myMsgBuf);
    mqd_t msqID = mq_open("/queue", O_RDONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);

//creating semaphore
    semID = sem_open("/sem", O_CREAT | O_EXCL, 0777, 1);

    //in clients nums are reversed!!!
    recMsgSem = sem_open("/recSem",  O_CREAT | O_EXCL, 0777, 0);

    sendMsgSem = sem_open("/sendSem",  O_CREAT | O_EXCL, 0777, 0);
int k;

//creating shared memory
    int fd = shm_open("mem",O_CREAT | O_RDWR, 0777);
    if(fd!=-1){
        if(ftruncate(fd, 3*sizeof(k)+sizeof(myPID))==0){
            shmAddr = mmap(NULL, 3*sizeof(k)+sizeof(myPID), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        }else{
            printf("err2");
            return 1;
        }
    }else{printf("err1");return 1;}

    shmAddr[0] = 1; //awake
    shmAddr[1] = stringToInt(argv[1]);  //queusize
    shmAddr[2] = 0; //clients in queue
    shmAddr[3] = myPID;


    struct myMsgBuf recMsg;

    while(1){

        //lock FIFO and shm
        sem_wait(semID);

        //check if anyone is waiting
        k=mq_receive(msqID,(char*) &recMsg, sizeof(struct myMsgBuf),NULL);
        if(k>0) {
            int sig;

            //one less client in queue
            shmAddr[2]--;


            //invite client for shaving
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"DEFAULT" invited\n", (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);
            kill(recMsg.pid, SIGUSR1);

            //receive confirmation - client is sitting in chair SIGUSR1
            sem_wait(recMsgSem);

            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"GREEN" is being shaved\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);

            //unlocking FIFO and shm
            sem_post(semID);

            //shaving itself:

            //letting client know shaving is complete
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\tClient "CYAN"%d"DARKRED" finished shaving\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec, recMsg.pid);
            sem_post(sendMsgSem);
            sem_wait(recMsgSem);


        }
        else{
            //go to sleep - no one in the queue
            clock_gettime(CLOCK_MONOTONIC, &tp);
            printf("[%d:%ld]\t"MAGENTA"Going to sleep\n"DEFAULT, (int) tp.tv_sec, tp.tv_nsec);

            //flag - asleep
            shmAddr[0] = 0;

            //open FIFO nad shm
            sem_post(semID);


            //don't do anything while asleep!
            while(!shmAddr[0]){
                ;
            }

        }

    }

    printf("Server stopped.\n");
    return 0;
}
