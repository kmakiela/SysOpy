//
// Created by kmakiela on 07.06.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>

#define search_equal      0
#define search_lesser     1
#define search_greater    2

#define print_all        0
#define print_cons       1

#define RED     "\x1b[91m"
#define YELLOW  "\x1b[93m"
#define BLU     "\x1b[34m"
#define CYAN    "\x1b[36m"
#define GREEN   "\x1b[32m"
#define DEFAULT "\x1b[0m"

int producers, consumers, buff_size, l, search_mode, print_mode, nk;
FILE* buff_source;

char** buffer = NULL;
int write_ind = 0, read_ind = 0, count = 0;

sem_t count_sem;
sem_t free_sem;
sem_t taken_sem;

void sigexit(int sig) {exit(EXIT_FAILURE);};
void err(const char* msg);
void load_config_file(char *const path);
void my_exit();
void consumer_l_printing(char *buff, int ind);

void* producer_fun(void *arg){
    char* buff = NULL;
    size_t n = 0;
    while (1){
        sem_wait(&free_sem);
        sem_wait(&count_sem);
        if (getline(&buff, &n, buff_source) <= 0){
            sem_post(&count_sem);
            break;
        }
        if (print_mode == print_all) printf(BLU"Producer puts one verse at  "CYAN"%i "GREEN"\tIndexes taken: "YELLOW"%i\n"DEFAULT, write_ind, count+1);
        buffer[write_ind] = buff;
        write_ind = (write_ind+1)%buff_size;
        count++;
        sem_post(&taken_sem);
        sem_post(&count_sem);
        n = 0;
        buff = NULL;
    }
    free(buff);
    pthread_exit((void*) 0);
}

void* consumer_fun(void *arg){
    char* buff;
    while (1){
        sem_wait(&taken_sem);
        sem_wait(&count_sem);
        buff = buffer[read_ind];
        buffer[read_ind] = NULL;
        if (print_mode == print_all) printf(RED"Consumer reads a verse from "CYAN"%i "GREEN"\tIndexes taken: "YELLOW"%i\n"DEFAULT, read_ind, count-1);
        consumer_l_printing(buff, read_ind);
        read_ind = (read_ind+1)%buff_size;
        count--;
        sem_post(&free_sem);
        sem_post(&count_sem);
        free(buff);
    }
    pthread_exit((void*) 0);
}

int main(int argc, char *argv[]){
    if (argc != 2) err("Wrong arguments");
    load_config_file(argv[1]);
    atexit(my_exit);
    signal(SIGINT, sigexit);
    sem_init(&count_sem, 0, 1);
    sem_init(&taken_sem, 0, 0);
    sem_init(&free_sem, 0, buff_size);
    pthread_t* prods = malloc(producers * sizeof(pthread_t));
    pthread_t* cons = malloc(consumers * sizeof(pthread_t));
    int i;
    buffer = malloc(buff_size * sizeof(char*));

    for (i = 0; i<producers; i++) if (pthread_create(&prods[i], NULL, &producer_fun, NULL)) err("Producer threads creation");
    for (i = 0; i<consumers; i++) if (pthread_create(&cons[i], NULL, &consumer_fun, NULL)) err("Consumer threads creation");

    if (nk) alarm(nk);
    for (i = 0; i<producers; i++) if (pthread_join(prods[i], NULL)) err("Producer threads joining");
    while (1){
        sem_wait(&count_sem);
        if (count == 0) break;
        sem_post(&count_sem);
    }
    exit(EXIT_SUCCESS);
}

void consumer_l_printing(char *buff, int ind){
    int flag;
    if (buff[strlen(buff)-1] == '\n') buff[strlen(buff)-1] = '\0';
    switch (search_mode){
        case search_equal:    flag = (strlen(buff) == l); break;
        case search_greater:  flag = (strlen(buff) > l);  break;
        case search_lesser:     flag = (strlen(buff) < l);  break;
    }
    if (flag) printf(CYAN"%i: "DEFAULT"%s\n", ind, buff);
}

void load_config_file(char *const path){
    FILE* conf;
    if ((conf = fopen(path, "r")) < 0) err("Configuration file");
    char buff[1024];
    fread(buff, 1024, 1, conf);
    char *p = strtok(buff, " "); //P,K,N,Name,L,szukanie,wypiwywanie,nk
    producers = atoi(p);
    p = strtok (NULL, " ");
    consumers = atoi(p);
    p = strtok (NULL, " ");
    buff_size = atoi(p);
    p = strtok (NULL, " ");
    if ((buff_source = fopen(p, "r")) == NULL) err("Source file");
    p = strtok (NULL, " ");
    l = atoi(p);
    p = strtok (NULL, " ");
    search_mode = (strcmp(p, "lesser") == 0) ? search_lesser : ((strcmp(p, "greater") == 0) ? search_greater : search_equal);
    p = strtok (NULL, " ");
    print_mode = (strcmp(p, "all") == 0) ? print_all : print_cons;
    p = strtok (NULL, " ");
    nk = atoi(p);
    if (producers <= 0 || consumers <= 0 || l < 0) err("Wrong P, K or L");
    if(fclose(conf) < 0) perror("Configuration file closing");
}

void err(const char* msg)
{
    if (errno) perror(msg);
    else printf(RED"%s\n"DEFAULT, msg);
    exit(EXIT_FAILURE);
}

void my_exit(){
    if (buffer) free(buffer);
    if (buff_source) fclose(buff_source);
    sem_destroy(&count_sem);
    sem_destroy(&free_sem);
    sem_destroy(&taken_sem);
}
