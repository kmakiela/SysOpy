#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    if(argc!=2){printf("Wrong number of arguments\n"); return 1;}
    FILE* fp;
    fp = fopen(argv[1],"r");
    if(!fp){printf("Bad filename\n"); return 1;}

    char arg_buffer[100];
    char* all_args[20];
    int arg_no;

    while(fgets(arg_buffer,100,fp)){
        arg_no = 0;
        while((all_args[arg_no] = strtok(arg_no == 0 ? arg_buffer: NULL," \n\t"))){
            arg_no++;
            if(arg_no>=20){printf("too many args\n"); return 1;}
        }
        pid_t pid;
        pid = fork();
        if(pid == 0){
            execvp(all_args[0],all_args);
            exit(0);
        }
        int w;
        wait(&w);
        if(w!=0){
            printf("error on function %s\n",all_args[0]);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}
