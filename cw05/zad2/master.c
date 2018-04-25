#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {printf("use ./master potok"); exit(EXIT_FAILURE);}
    if (mkfifo(argv[1], S_IRWXU | S_IRWXG | S_IRWXO) < 0){
        if (errno != EEXIST){printf("Bad master pipe");exit(EXIT_FAILURE);}
    }

    int fd, count;
    char bufor[1000];
    if ((fd = open(argv[1], O_RDONLY)) < 0) {printf("Bad master pipe");exit(EXIT_FAILURE);}
    while ((count = read(fd, bufor, 1000)) > 0){
        printf("%.*s",count, bufor);
	strcpy(bufor,"");
    }
    close(fd);
    remove(argv[1]);
    printf("Master finished\n");
    exit(EXIT_SUCCESS);
}
