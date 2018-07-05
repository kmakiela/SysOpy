#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
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
mqd_t server;
int client_id = -1;
char queue_name[8];

void myexit();
void err(const char* msg);
void sig_handler(int sig);
void set_sigint();
char parse_command(char *line, int len, int* pos);
void connect_to_server(char* msg);
void receive_reply(char* msg);

int main(int argc, char const *argv[]){
    FILE *file;
    int intr = 1;
    if (argc > 1){
        file = fopen(argv[1], "r");
        if (file == NULL) err("Commands file");
        intr = 0;
    }
    else file = stdin;
    set_sigint();
    atexit(myexit);
    char *buf = NULL;
    char line[100];
    size_t n;
    char msg[MAX_MSG];
    int count, pos;
    char msg_id;
    sprintf(queue_name, "/%iq", getpid());

    if ((server = mq_open(SERVER_NAME, O_WRONLY)) < 0) err("Client to server queue");
    if ((msgqueue = mq_open(queue_name, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, NULL)) < 0) err("Server to client queue");

    connect_to_server(queue_name);
    while (printf("In: ") < 0 || (count = getline(&buf, &n, file)) > 1){
        sprintf(line, "%.*s", (buf[count-1] == '\n') ? count-1 : count, buf);
        msg_id = parse_command(line, count, &pos);
        if (msg_id == UNDEF_MSG){
            printf("Wrong command %s\n", line);
            continue;
        }
        msg[0] = msg_id;
        sprintf(msg+2, "%s", line + pos);
        msg[1] = (char) client_id;
        if (mq_send(server, msg, MAX_MSG, 1) < 0){
            if (errno == EBADF && intr){
                printf("Server not found\n");
                continue;
            }
            else err("Client send");
        }
        receive_reply(msg);
    }
    printf("Exit with success\n");
    exit(EXIT_SUCCESS);
}

char parse_command(char *line, int len, int* pos){
    int c = 0;
    int ret;
    char cmnd[6];
    while (line[c] != ' ' && c != len-1 && c < 6) c++;
    sprintf(cmnd, "%.*s", c, line);
    if (line[c] == ' ') while(line[c] == ' ') c++;
    *pos = c;
    if (strcmp(cmnd, "MIRROR") == 0)
        ret = MIRROR_MSG;
    else if (strcmp(cmnd, "CALC") == 0)
        ret = CALC_MSG;
    else if (strcmp(cmnd, "TIME") == 0)
        ret = TIME_MSG;
    else if (strcmp(cmnd, "END") == 0)
        ret = END_MSG;
    else ret = UNDEF_MSG;
    return ret;
}

void connect_to_server(char* queue_name){
    char msg[MAX_MSG];
    msg[0] = INIT_MSG;
    msg[1] = -1;
    sprintf(msg+2, "%s", queue_name);
    if (mq_send(server, msg, MAX_MSG, 1) < 0) err("Client init");
    if (mq_receive(msgqueue, msg, MAX_MSG, NULL) < 0) err("Client init");
    switch(msg[0])
    {
        case RPLY_MSG:
            client_id = atoi(msg+2);
            if (client_id < 0) err("Client register");
            break;
        case ERR_MSG:
            err(msg+2);
            break;
        default:
            break;
    }
}

void receive_reply(char* msg){
    if (msg[0] == END_MSG) return;
    if (mq_receive(msgqueue, msg, MAX_MSG, NULL) < 0) perror("Client receive");
    printf("%s\n", msg+2);
}

void myexit(void){
    if (client_id != -1){
        char msg[MAX_MSG];
        msg[0] = STOP_MSG;
        msg[1] = (char)client_id;
        sprintf(msg+2, "%i", client_id);
        if (mq_send(server, msg, MAX_MSG, 1) < 0) perror("Exit send");
    }
    if (mq_close(server) < 0) perror("Exit close server");
    if (mq_close(msgqueue) < 0) perror("Exit close client");
    if (mq_unlink(queue_name) < 0) perror("Exit unlink client");
}

void err(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(EXIT_FAILURE);
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