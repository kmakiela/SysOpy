#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>

int toggle = 1;
pid_t child_pid;

void handlingSTP(int sig){
    if(toggle){printf("\nOdebrano: %d Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n",sig);}
    toggle = toggle==1 ? 0:1;
}

void handlingINT(int sig){
    printf("\nOdebrano: %d\n",sig);
    exit(EXIT_SUCCESS);
}

int main() {
    struct sigaction action;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    action.sa_handler = (&handlingSTP);
    int is_dead=0;

    child_pid = fork();
    if(child_pid==0){
        execl("./dates","./dates",NULL);
        exit(EXIT_SUCCESS);
    }

    while(1){
        sigaction(SIGTSTP,&action,NULL);
        signal(SIGINT,&handlingINT);

        if(toggle) {
            if(is_dead){
                is_dead = 0;
                child_pid = fork();
                if(child_pid==0){
                    execl("./dates","./dates",NULL);
                    exit(EXIT_SUCCESS);
                }

            }
        }
        if(!toggle){
            if(!is_dead){
                is_dead = 1;
                kill(child_pid,SIGKILL);
            }

        }
    }
}