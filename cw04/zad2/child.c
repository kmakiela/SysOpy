#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void usrhandler(int sig){
    kill(getppid(), SIGRTMIN+(rand()%(SIGRTMAX-SIGRTMIN+1)));
}

int main(){
    //obsluga sygnalu od rodzica
    signal(SIGUSR1, usrhandler);
    //zablokowanie sygnalow innych niz SIGUSR1
    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGUSR1);
    //spanie
    srand((uint)getppid()*getpid()*getpid());
    uint secs;
    secs = rand()%11;
    sleep(secs);
    //wyslanie sygnalu o gotowości i czekanie na odpowiedz
    kill(getppid(),SIGUSR1);
    sigsuspend(&mask);

    return secs;
}