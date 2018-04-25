#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

int charToInt(const char* arg){
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

int main(int argc, char const *argv[]){
    if (argc != 3){printf("Wrong arguments.");exit(EXIT_FAILURE);}
    int n = charToInt(argv[2]);
    if (n <= 0) {printf("Incorrect arguments.");exit(EXIT_FAILURE);}
    int fd, i, seed;
    char datebufor[100];
    char bufor[1000];
    time_t what;
    FILE *date;

    seed = time(&what);
    srand(seed + getpid());
    if ((fd = open(argv[1], O_WRONLY)) < 0){printf("Bad slave pipe\n");exit(EXIT_FAILURE);}
    for (i = 0; i < n; i++){
        date = popen("date", "r");
        if (date == NULL || date < 0){printf("Wrong date\n");exit(EXIT_FAILURE);}
        fgets(datebufor, 1000, date);
        pclose(date);

        sprintf(bufor,"Slave %i:\t%d/%d\t%s\n", getpid(),i+1,n, datebufor);
        if (i == n-1) sprintf(bufor,"%sSlave %i finished\n",bufor,getpid());
        lseek (fd, 0, SEEK_SET);
        if (write(fd, bufor, strlen(bufor)) <= 0){printf("pipe error again\n");exit(EXIT_FAILURE);}
        sleep(rand()%4 + 2);
    }
    close (fd);

    exit(EXIT_SUCCESS);
}
