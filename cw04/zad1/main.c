#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int toggle = 1;

void handlingSTP(int sig){
    if(toggle){printf("\nOdebrano: %d Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n",sig);}
    toggle = toggle==1 ? 0:1;
}

void handlingINT(int sig){
    printf("\nOdebrano: %d\n",sig);
    exit(EXIT_SUCCESS);
}

int main() {
    struct tm* date;
    time_t rawtime;
    struct sigaction action;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = (&handlingSTP);

    sigaction(SIGTSTP,&action,NULL);
    signal(SIGINT,&handlingINT);

    while(1){
        if(toggle) {
            time(&rawtime);
            date = localtime(&rawtime);
            printf("\nCzas: %s", asctime(date));
            sleep(1);
        }
    }
}