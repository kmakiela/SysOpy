#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

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

void set_limits(char* time, char* mem){
    long int tlim = charToInt(time);
    struct rlimit rlimit_time;
    rlimit_time.rlim_cur = (rlim_t) tlim;
    rlimit_time.rlim_max = (rlim_t) tlim;
    int retval;
    retval = setrlimit(RLIMIT_CPU,&rlimit_time);
    if(retval!=0){printf("Błąd ustawiania limitu czasu");return;}

    long int memory = charToInt(mem)*1024*1024;
    struct rlimit rlimit_mem;
    rlimit_mem.rlim_cur = (rlim_t) memory;
    rlimit_mem.rlim_max = (rlim_t) memory;
    int retval2;
    retval2 = setrlimit(RLIMIT_AS,&rlimit_mem);
    if(retval2!=0){printf("Błąd ustawiania limitu pamięci");return;}
}

int main(int argc, char *argv[]) {
    if(argc!=4){printf("Wrong number of arguments\n"); return 1;}
    FILE* fp;
    fp = fopen(argv[1],"r");
    if(!fp){printf("Bad filename\n"); return 1;}

    char arg_buffer[100];
    char* all_args[20];
    int arg_no;
    struct rusage start;

    while(fgets(arg_buffer,100,fp)){
        arg_no = 0;
        while((all_args[arg_no] = strtok(arg_no == 0 ? arg_buffer: NULL," \n\t"))){
            arg_no++;
            if(arg_no>=20){printf("too many args\n"); return 1;}
        }
        getrusage(RUSAGE_CHILDREN,&start);
        pid_t pid;
        pid = fork();
        if(pid == 0){
            set_limits(argv[2],argv[3]);
            execvp(all_args[0],all_args);
            exit(0);
        }
        int w;
        wait(&w);
        if(w!=0){
            printf("error on function %s\n",all_args[0]);
            return 1;
        }
        struct rusage end;
        getrusage(RUSAGE_CHILDREN,&end);
        struct timeval utime;
        struct timeval stime;
        timersub(&end.ru_utime, &start.ru_utime,&utime);
        timersub(&end.ru_stime, &start.ru_stime,&stime);
        printf("User time: %ld.%lds \t System time: %ld.%lds\n",(long int) utime.tv_sec,(long int) utime.tv_usec,(long int) stime.tv_sec,(long int) stime.tv_usec);

    }
    fclose(fp);
    return 0;
}
