#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_WRD 100
#define MAX_ARG 100

typedef struct words
{
    char *list[MAX_WRD];
    size_t length;
} words;

typedef struct arg_list
{
    words wlist[MAX_ARG];
    size_t length;
} arg_list;

words getwords(const char *text){
    int i, count = 0, index_start_word = 0, new_word = 0;
    size_t text_len = strlen(text);
    words wl;

    for (i = 0; i <= text_len; i++){
        if (isspace(text[i]) || i == text_len){
            if (new_word){
                wl.list[count] = strndup(text + index_start_word, i - index_start_word);
                new_word = 0;
                count++;
            }
        }
        else if (!new_word){
            new_word = 1;
            index_start_word = i;
        }
    }
    wl.list[count] = NULL;
    wl.length = count;

    return wl;
}

arg_list parse_line(const char *text){
    words wl = getwords(text);
    arg_list el;
    int i, j = 0, k = 0, lastw = 0;
    for (i = 0; i < wl.length; i++){
        if (strcmp(wl.list[i], "|") == 0){
            if (i > lastw){
                el.wlist[j].length = i - lastw;
                el.wlist[j].list[k] = NULL;
                k = 0;
                j++;
            }
            lastw = i + 1;
            continue;
        }
        el.wlist[j].list[k] = wl.list[i];
        k++;
    }
   if (i > lastw){
   	el.wlist[j].length = i - lastw;
   	el.wlist[j].list[k] = NULL;
   	j++;
   }
    el.length = j;

    return el;
}

void err(const char* msg)
{
    if (errno) perror(msg);
    else printf("%s\n", msg);
    killpg(getpgrp(), SIGINT);
}

int main(int argc, char const *argv[])
{
    if (argc!=2) err("Bad args, use ./main pol");
    int i;
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    int sl, count = 0;
    int fd[MAX_ARG - 1][2];
    pid_t child_pid[MAX_ARG];
    arg_list el;

    if ((fp = fopen(argv[1], "r")) == NULL) err(argv[1]);

    while (getline(&line, &len, fp) != -1){
        el = parse_line(line);
	printf("\nInput line:\t%s\n",line);
        for (i = 0; i < el.length; i++){
            if ((pipe(fd[i])) < 0) err(el.wlist[i].list[0]);
            if ((child_pid[i] = fork()) < 0) err(line);
            if (child_pid[i] == 0){
                if (i){
                    dup2(fd[i-1][0], STDIN_FILENO);
                    close(fd[i-1][1]);
                }
                if (i != el.length-1){
                    dup2(fd[i][1], STDOUT_FILENO);
                    close(fd[i][0]);
                }
                execvp(el.wlist[i].list[0], el.wlist[i].list);
                exit(EXIT_FAILURE);
            }
            if (i) close(fd[i-1][0]);
            if (i != el.length-1) close(fd[i][1]);            
        }
        for (i = 0; i < el.length; i++){
            if (waitpid(child_pid[i], &sl, 0) < 0) err(line);
            if (!WIFEXITED(sl) || WEXITSTATUS(sl)){
                printf("Command %i stopped\n", count);
                break;
            }
        }
        if (errno) break;
    }
    if (line != NULL) free(line);
    fclose(fp);
    if (errno) exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}
