#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "msglib.h"

int msgqueue;
void err(const char* msg);
void snd_err(const char* msgtext, int msgqid);
void sig_handler(int sig);
void set_sigint();
void init_client(int *clients, struct msgbuf* msg);
void delete_client(int *clients, struct msgbuf* msg);
void handle_mirror(int client_queue_id, char* msg_text);
void handle_calc(int clqid, char* msg, int len);
void handle_time(int client_queue_id);
void myexit(void){msgctl(msgqueue, IPC_RMID, NULL);}

int main(int argc, char const *argv[])
{
    const char *rqsts[4] = {"MIRROR", "CALC", "TIME", "END"};
    int i, break_flag = 0;
    struct msgbuf msg;
    int clients[MAX_CLIENTS];
    for (i = 0; i < MAX_CLIENTS; i++) clients[i] = -1;

    set_sigint();
    atexit(myexit);
    if ((msgqueue = msgget(ftok(getenv("HOME"), 0), IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) < 0) err("Creating queue");
    for (;;){
        if (msgrcv(msgqueue, &msg, MAX_MSG, 0, MSG_NOERROR) < 0) err("Receiving message");
        if (msg.mtype < 5) printf("Received command %s from #%i\n", rqsts[msg.mtype-1], msg.client_id);
        switch (msg.mtype)
        {
            case INIT_MSG:
                init_client(clients, &msg);
                break;
            case STOP_MSG:
                delete_client(clients, &msg);
                break;
            case MIRROR_MSG:
                handle_mirror(clients[msg.client_id], msg.mtext);
                break;
            case CALC_MSG:
                handle_calc(clients[msg.client_id], msg.mtext, strlen(msg.mtext));
                break;
            case TIME_MSG:
                handle_time(clients[msg.client_id]);
                break;
            case END_MSG:
                break_flag = 1;
                break;
            
            default:
                printf("Wrong command %s\n", msg.mtext);
                break;
        }
        if (break_flag) break;
    }
    printf("Exit with success\n");
    exit(EXIT_SUCCESS);
}

void init_client(int *clients, struct msgbuf* msg){
    int i, tmp;
    tmp = atoi(msg->mtext);
    for (i = 0; i < MAX_CLIENTS && clients[i] != -1; i++) {;}
    if (i < MAX_CLIENTS){
        clients[i] = tmp;
        msg->mtype = RPLY_MSG;
        sprintf(msg->mtext, "%i", i);
        if (msgsnd(clients[i], msg, MAX_MSG, 0) < 0){
            perror("Bad client initalization");
            clients[i] = -1;
            return;
        }
        printf("Client %i initialized with pid %i\n", i, msg->client_pid);
    }
    else{
        msg->mtype = ERR_MSG;
        sprintf(msg->mtext, "Maximum number of clients reached\n");
        if (msgsnd(tmp, msg, MAX_MSG, 0)) perror("Too many clients error");
    }
}

void delete_client(int *clients, struct msgbuf* msg){
    if (msg->client_id < 0 || msg->client_id > MAX_CLIENTS) return;
    printf("Client %i with pid %i deleted\n", msg->client_id, msg->client_pid);
    clients[msg->client_id] = -1;
}

void handle_mirror(int client_id, char* text){
    char *p1 = text;
    char *p2 = text + strlen(text) - 1;
    while (p1 < p2){
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    struct msgbuf msg;
    msg.mtype = RPLY_MSG;
    sprintf(msg.mtext, "%s", text);
    if (msgsnd(client_id, &msg, MAX_MSG, 0) < 0) perror("Mirror handler");
}

void handle_calc(int client_id, char* msg, int len){
    char opr, tmp[10];
    int c = 0, n1, n2, res;
    while (msg[c] > '0' && msg[c] <= '9' && c < len) c++;
    if (c == 0) {snd_err("Bad arithmetic expression\n", client_id); return;}
    sprintf(tmp, "%.*s",(c > 10) ? c : 10, msg);
    n1 = atoi(tmp);
    while (msg[c] == ' ' && c < len) c++;
    if (c == len) {snd_err("Bad arithmetic expression\n", client_id); return;}
    opr = msg[c++];
    while (msg[c] == ' ' && c < len) c++;
    if (c == len || msg[c] < '0' || msg[c] > '9') {snd_err("Bad arithmetic expression\n", client_id); return;}
    sprintf(tmp, "%.*s", (len-c > 10) ? len-c : 10, msg + c);
    n2 = atoi(tmp);
    switch (opr)
    {
        case '+': res = n1 + n2; break;
        case '-': res = n1 - n2; break;
        case '/': if (n2) res = n1 / n2; else {snd_err("I'm not gonna fall for that\n", client_id); return;} break;
        case '*': res = n1 * n2; break;
        default: snd_err("Calc handler", client_id); return;
    }
    struct msgbuf reply;
    reply.mtype = RPLY_MSG;
    sprintf(reply.mtext, "%i", res);
    if (msgsnd(client_id, &reply, MAX_MSG, 0) < 0) perror("Calc reply\n");
}

void handle_time(int id){
    struct msgbuf reply;
    reply.mtype = RPLY_MSG;

    FILE *date = popen("date", "r");
    if (date == NULL || date < 0) perror("date");
    fgets(reply.mtext, MAX_MSG_TXT, date);
    pclose(date);
    sprintf(reply.mtext, "%.*s", (int)strlen(reply.mtext)-1, reply.mtext);
    if (msgsnd(id, &reply, MAX_MSG, 0) < 0) perror("time reply");
}

void err(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

void snd_err(const char* msgtext, int id){
    struct msgbuf msg;
    msg.mtype = ERR_MSG;
    sprintf(msg.mtext,"%s", msgtext);
    msgsnd(id, &msg, MAX_MSG, 0);
}

void sig_handler(int sig){
    if (sig == SIGINT){
        printf("\nGunned down by SIGINT\n");
        exit(EXIT_SUCCESS);
    }
    else if (sig == SIGSEGV) err("Segmentation fault");
}

void set_sigint(){
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < -1) err("Signal");
    if (sigaction(SIGSEGV, &act, NULL) < -1) err("Signal");
}
