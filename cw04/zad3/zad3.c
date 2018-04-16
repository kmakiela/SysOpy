#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

int sentToChild=0;
volatile int recByChild;
int recByPar=0;
volatile int L=0;

int stringToInt(char* s){
    int result = 0;
    for(int i=0; i<strlen(s); i++){
        result*=10;
        result+=(s[i]-'0');
    }
    return result;
}


void catchSIGINT(int signo){
    printf("\nOdebrano sygnal SIGINT\n");
    kill(0, SIGUSR2);
    kill(0, SIGRTMAX);
//    printf("Sent: %d\nReceived by child: %d\nReceived by parent: %d\n", sentToChild, recByChild, recByPar);
    exit(0);
}


void catchChildUSR1(int signo, siginfo_t* siginfo, void* context){
    recByChild++;
//    printf("rec: %d\n", recByChild);
	union sigval value;
    sigqueue(siginfo->si_pid, signo, value);
//   printf("cought usr1\n");
}

void catchParentUSR1(int signo, siginfo_t* siginfo, void* context){
    recByPar++;
//    printf("coucht usr1 parent\n");
}

void catchParent2USR1(int signo, siginfo_t* siginfo, void* context){
    recByPar++;
    sentToChild++;
    if(sentToChild!=L)
    kill(siginfo->si_pid, signo);
//    printf("coucht usr1 parent\n");
}

void catchUSR2(int signo, siginfo_t* siginfo, void* context){
//    printf("coucht usr2\n");
    printf("Received by child: %d\n", recByChild);
    exit(recByChild);
}


void catchSIGCHLD(int signo, siginfo_t* siginfo, void* context){
    sleep(1);
    int status;
    int cpid = wait(&status);
    if(WIFEXITED(status) && cpid==siginfo->si_pid){
    //printf("Process %d exited with status %d.\n", siginfo->si_pid, WEXITSTATUS(status));}
    printf("Sent by parent: %d\nReceived by parent: %d\n", sentToChild, recByPar);}
    else{printf("child returned\n");}
exit(0);
}

int main(int argc, char* argv[]){
    if(argc!=3){printf("Wrong argc.\n"); return 1;}

    L = stringToInt(argv[1]);
    int Type = stringToInt(argv[2]);
    int childPID=-1;
    recByChild=0;

    if(Type==1){
        struct sigaction actUSR1;
        actUSR1.sa_flags = SA_SIGINFO;
        actUSR1.sa_sigaction = catchChildUSR1;
        sigaction(SIGUSR1, &actUSR1, NULL);

        struct sigaction actUSR2;
        actUSR2.sa_flags = SA_SIGINFO;
        actUSR2.sa_sigaction = catchUSR2;
        sigaction(SIGUSR2, &actUSR2, NULL);

        childPID = fork();
        if(childPID==0){
            while(1){
                pause();
            }
        }
        else{
            actUSR1.sa_sigaction = catchParentUSR1;
            sigaction(SIGUSR1, &actUSR1, NULL);
            struct sigaction actINT;
            actINT.sa_handler = catchSIGINT;
            sigaction(SIGINT, &actINT, NULL);

            struct sigaction actSIGCHLD;
            actSIGCHLD.sa_flags = SA_SIGINFO;
            actSIGCHLD.sa_sigaction = catchSIGCHLD;
            sigaction(SIGCHLD, &actSIGCHLD, NULL);

            for(int i=0; i<L; i++){
               if(kill(childPID,SIGUSR1)==0) {
                    //printf("sentusr1\n");
                   sentToChild++;}
            //    printf("%d\n", i);
            }
            
            kill(childPID, SIGUSR2);
        }
    }
    else{
        if(Type==3){
            struct sigaction actUSR1;
            actUSR1.sa_flags = SA_SIGINFO;
            actUSR1.sa_sigaction = catchChildUSR1;
            sigaction(SIGRTMIN, &actUSR1, NULL);

            struct sigaction actUSR2;
            actUSR2.sa_flags = SA_SIGINFO;
            actUSR2.sa_sigaction = catchUSR2;
            sigaction(SIGRTMAX, &actUSR2, NULL);
            
            union sigval value;

            childPID = fork();
            if(childPID==0){
            
                while(1){
                    pause();
                }
            }
            else{
                actUSR1.sa_sigaction = catchParentUSR1;
                sigaction(SIGRTMIN, &actUSR1, NULL);
            
                struct sigaction actINT;
                actINT.sa_handler = catchSIGINT;
                sigaction(SIGINT, &actINT, NULL);

                struct sigaction actSIGCHLD;
                actSIGCHLD.sa_flags = SA_SIGINFO;
                actSIGCHLD.sa_sigaction = catchSIGCHLD;
                sigaction(SIGCHLD, &actSIGCHLD, NULL);
				
                for(int i=0; i<L; i++){
                    if(sigqueue(childPID,SIGRTMIN, value)==0) {
                      //  printf("sentusr1\n"); 
                        sentToChild++;}
              //      printf("%d\n", i);
                }
                while(recByPar<L);
                
                
                kill(childPID, SIGRTMAX);
            }
        }
        else{
            if(Type==2){
                
                struct sigaction actUSR1;
                actUSR1.sa_flags = SA_SIGINFO;
                actUSR1.sa_sigaction = catchChildUSR1;
                sigaction(SIGUSR1, &actUSR1, NULL);

                struct sigaction actUSR2;
                actUSR2.sa_flags = SA_SIGINFO;
                actUSR2.sa_sigaction = catchUSR2;
                sigaction(SIGUSR2, &actUSR2, NULL);

                childPID = fork();
                if(childPID==0){
                    //printf("child\n");
                    while(1){
                        pause();
                    //sleep(1);
                    }
                }
                else{
                //sleep(5);
                actUSR1.sa_sigaction = catchParent2USR1;
                sigaction(SIGUSR1, &actUSR1, NULL);
            
                struct sigaction actINT;
                actINT.sa_handler = catchSIGINT;
                sigaction(SIGINT, &actINT, NULL);

                struct sigaction actSIGCHLD;
                actSIGCHLD.sa_flags = SA_SIGINFO;
                actSIGCHLD.sa_sigaction = catchSIGCHLD;
                sigaction(SIGCHLD, &actSIGCHLD, NULL);
                
                //sleep(2);
                //for(int i=0; i<L; i++){
                //printf("sendingUSR1\n");
                //sentToChild++;
                if(kill(childPID,SIGUSR1)!=0)
                {printf("Err\n"); return 1;}
                while(sentToChild!=L){
                //     pause();
                     sleep(1);
                }

                //printf("%d\n", i);
        
                //}
                //sleep(1);
                kill(childPID, SIGUSR2);
                }
            }
            else{
                printf("Wrong arguments.\n");
                return 1;
            }
        }
    }
    while(1){
        pause();
    }

    return 0;
}
