#include <mqueue.h>
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
#include "posix.h"

mqd_t msgqueue;
mqd_t clients[MAX_CLIENTS];
void err(const char* msg);
void snd_err(const char* msgtext, mqd_t msgqid);
void sig_handler(int sig);
void set_sigint();
void init_client(char *msg);
void delete_client(char *msg);
void handle_mirror(mqd_t queue_id, char *msg_text);
void handle_calc(mqd_t clqid, char *msg, int len);
void handle_time(mqd_t queue_id);
void myexit(void);

int main(int argc, char const *argv[]){
    const char *rqsts[4] = {"MIRROR", "CALC", "TIME", "END"};
    int i, break_flag = 0;
    char type, msg[MAX_MSG];
    for (i = 0; i < MAX_CLIENTS; i++) clients[i] = (mqd_t)-1;
    set_sigint();
    atexit(myexit);

    if ((msgqueue = mq_open(SERVER_NAME, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, NULL)) < 0) err("Server queue open");
    for (;;){
        if (mq_receive(msgqueue, msg, MAX_MSG, NULL) < 0) err("Server receive");
        type = msg[0];
        if (type < 5) printf("Received command %s from #%i\n", rqsts[type-1], (int)msg[1]);
        switch (type)
        {
            case INIT_MSG:
                init_client(msg);
                break;
            case STOP_MSG:
                delete_client(msg);
                break;
            case MIRROR_MSG:
                handle_mirror((int) clients[(int) msg[1]], msg + 2);
                break;
            case CALC_MSG:
                handle_calc((int) clients[(int) msg[1]], msg + 2, strlen(msg + 2));
                break;
            case TIME_MSG:
                handle_time((int) clients[(int) msg[1]]);
                break;
            case END_MSG:
                break_flag = 1;
                break;
            
            default:
                printf("Wrong command %s\n", msg + 2);
                break;
        }
        if (break_flag) break;
    }
    printf("Exit with success\n");
    exit(EXIT_SUCCESS);
}

void init_client(char *msg){
    int i;
    char tmp_name[MAX_MSG-2];
    mqd_t tmp;
    sprintf(tmp_name, "%s", msg+2);
    if ((tmp = mq_open(tmp_name, O_WRONLY)) < 0) err("server to client queue open");
    for (i = 0; i < MAX_CLIENTS && clients[i] != -1; i++) {;}
    if (i < MAX_CLIENTS){
        clients[i] = tmp;
        msg[0] = RPLY_MSG;
        sprintf(msg+2, "%i", i);
        if (mq_send(clients[i], msg, MAX_MSG, 1) < 0){
            perror("Connection init");
            clients[i] = (mqd_t) -1;
            return;
        }
        printf("Client %i initialized\n", i);
    }
    else{
        msg[0] = ERR_MSG;
        sprintf(msg+2, "Maximum number of clients reached");
        if (mq_send(tmp, msg, MAX_MSG, 1)) perror("Init client");
    }
}

void delete_client(char *msg){
    if (msg[1] < 0 || msg[1] > MAX_CLIENTS) return;
    int client_id = (int)msg[1];
    if (mq_close(clients[client_id]) < 0) perror("Delete client");
    printf("Client %i deleted\n", client_id);
    clients[(int)msg[1]] = -1;
}

void handle_mirror(mqd_t queue_id, char *msg_text){
    char *p1 = msg_text;
    char *p2 = msg_text + strlen(msg_text) - 1;
    while (p1 < p2){
        char tmp = *p1;
        *p1++ = *p2;
        *p2-- = tmp;
    }
    char msg[MAX_MSG];
    msg[0] = RPLY_MSG;
    sprintf(msg+2, "%s", msg_text);
    if (mq_send(queue_id, msg, MAX_MSG, 1) < 0) perror("Server send");
}

void handle_calc(mqd_t clqid, char *msg, int len){
    char opr, tmp[10];
    int c = 0, n1, n2, res;
    while (msg[c] > '0' && msg[c] <= '9' && c < len) c++;
    if (c == 0) {snd_err("Bad arithmetic expression", clqid); return;}
    sprintf(tmp, "%.*s",(c > 10) ? c : 10, msg);
    n1 = atoi(tmp);
    while (msg[c] == ' ' && c < len) c++;
    if (c == len) {snd_err("Bad arithmetic expression", clqid); return;}
    opr = msg[c++];
    while (msg[c] == ' ' && c < len) c++;
    if (c == len || msg[c] < '0' || msg[c] > '9') {snd_err("Bad arithmetic expression", clqid); return;}
    sprintf(tmp, "%.*s", (len-c > 10) ? len-c : 10, msg + c);
    n2 = atoi(tmp);
    switch (opr)
    {
        case '+': res = n1 + n2; break;
        case '-': res = n1 - n2; break;
        case '/': if (n2) res = n1 / n2; else {snd_err("Bad arithmetic expression", clqid); return;} break;
        case '*': res = n1 * n2; break;
        default: snd_err("Bad arithmetic expression", clqid); return;
    }
    char msgs[MAX_MSG];
    msgs[0] = RPLY_MSG;
    sprintf(msgs+2, "%i", res);
    if (mq_send(clqid, msgs, MAX_MSG, 1) < 0) perror("Server send");
}

void handle_time(int client_queue_id){
    char msg[MAX_MSG];
    msg[0] = RPLY_MSG;
    FILE *date = popen("date", "r");
    if (date == NULL || date < 0) perror("date");
    fgets(msg+2, MAX_MSG-2, date);
    pclose(date);
    sprintf(msg+2, "%.*s", (int)strlen(msg+2)-1, msg+2);
    if (mq_send(client_queue_id, msg, MAX_MSG, 1) < 0) perror("Server send");
}

void err(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(EXIT_FAILURE);
}

void snd_err(const char* msgtext, mqd_t msgqid){
    char msg[MAX_MSG];
    msg[0] = ERR_MSG;
    sprintf(msg+2,"%s", msgtext);
    if(mq_send(msgqid, msg, MAX_MSG, 1) < 0) perror("Error send");
}

void sig_handler(int sig){
    if (sig == SIGINT){
        printf("\nGunned down by SIGINT\n");
        exit(EXIT_SUCCESS);
    }
}

void set_sigint(){
    struct sigaction act;
    act.sa_handler = sig_handler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < -1) err("Signal");
}

void myexit(void){
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
        if (clients[i] != -1) if (mq_close(clients[i]) < 0) perror("Client remove");
    mq_close(msgqueue);
    mq_unlink(SERVER_NAME);
}
