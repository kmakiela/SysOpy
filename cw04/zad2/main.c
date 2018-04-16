#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <errno.h>

int N,K,k,n;

pid_t* parents;
pid_t* waiting;

int charToInt(char* arg){
    int i;
    int liczba=0;
    for(i=strlen(arg)-1;i>=0;i--){
        int j = strlen(arg)-1-i;
        int k,tmp=1;
        for(k=1;k<=j;k++){
            tmp*=10;
        }
        liczba +=(arg[i]-'0')*tmp;
    }
    return liczba;
}

int is_in_parents(pid_t pid){
    int i;
    for(i=0;i<N;i++){
        if(parents[i]==pid){return i;}
    }
    return -1;
}

void remove_child(pid_t pid) {
    for (int i = 0; i < N; i++)
        if (parents[i] == pid) {
            parents[i] = -1;
            return;
        }
}

void usr_handler(int sig,siginfo_t* info,void* ucontext) {
    printf("Received SIGUSR1 from pid = %d\n", info->si_pid);
    //wszechmatka
    if(is_in_parents(info->si_pid) == -1){return;}
    //rodzice
    if(k>=K){
        printf("Parent immediately sends permission to child %d to send RT signal back\n",info->si_pid);
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
    }
    if(k < K){
        waiting[k] = info->si_pid;
        k+=1;
        if(k>=K){
            printf("Achieved required amount of requests = %d\n",K);
            int i;
            for(i=0;i<K;i++){
                printf("Parent sends permission to child %d to send RT signal back\n",waiting[i]);
                kill(waiting[i],SIGUSR1);
                waitpid(waiting[i], NULL,0);
            }
        }
    }
}

void rt_handler(int sig,siginfo_t* info,void* ucontext){
    printf("Received SIGRTMIN+%d from pid = %d\n",sig-SIGRTMIN,info->si_pid);
}

void chld_handler(int sig, siginfo_t* info, void* ucontext){
    if(info->si_code == CLD_EXITED) {
        printf("Child with pid = %d slept for %d seconds\n", info->si_pid, info->si_status);
    }
    remove_child(info->si_pid);
    n--;
    if(n==0){
        printf("No children left, terminating\n");
        free(parents);
        free(waiting);
        exit(0);
    }
}

void int_handler(int sig, siginfo_t* info, void* ucontext){
    printf("\nReceived SIGINT\n");
    int i;
    for (i = 0; i < N; i++)
        if (parents[i] != -1) {
            kill(parents[i], SIGKILL);
            waitpid(parents[i], NULL, 0);
        }
    free(parents);
    free(waiting);
    printf("Zabito wszystkie procesy\n");
    exit(0);
}

int main(int argc, char* argv[]) {
    if(argc!=3){printf("Wrong use of program, use ./main N K\n"); return 1;}
    N = charToInt(argv[1]);
    K = charToInt(argv[2]);
    k=0;
    n=0;
    if(N<1 || K<1 || N<K){printf("Bad arguments\n");return 1;}
    //potrzebne dane
    parents = malloc(N*sizeof(pid_t));
    waiting = malloc(K*sizeof(pid_t));
    struct sigaction action;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_SIGINFO | SA_NODEFER | SA_NOCLDSTOP;
    int i;

    //handling USR1 from children
    action.sa_sigaction = usr_handler;
    int r = sigaction(SIGUSR1,&action,NULL);
    if(r==-1){printf("Error processing SIGUSR1\n");return 1;}
    //handling RT back from children
    action.sa_sigaction = rt_handler;
    for(i=SIGRTMIN;i<=SIGRTMAX;i++){
        int r2 = sigaction(i,&action,NULL);
        if(r2==-1){printf("Error processing SIGRTMIN+%d\n",i);return 1;}
    }
    //handling SIGCHLD
    action.sa_sigaction = chld_handler;
    int r3 = sigaction(SIGCHLD,&action,NULL);
    if(r3==-1){printf("Error processing SIGCHLD\n");return 1;}
    //handling INT
    action.sa_sigaction = int_handler;
    int r4 = sigaction(SIGINT,&action,NULL);
    if(r4==-1){printf("Error processing SIGINT\n");return 1;}

    //tworzenie dzieci
    for(i=0;i<N;i++){
        pid_t parent= fork();
        if(!parent){
            execl("./child", "./child",NULL);
            printf("Something wrong with creating child\n");
        }
        else{
            printf("Stworzono proces o pid = %d\n",(int)parent);
            parents[n] = parent;
            n++;
        }
    }
    while(wait(NULL)){
        if(errno==ECHILD){printf("No children left, terminating\n");exit(1);}
    }

}
