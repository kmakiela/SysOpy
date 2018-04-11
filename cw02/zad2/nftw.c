#define _XOPEN_SOURCE 700
#include <time.h>
#include <ftw.h>
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

//zmienne globalne
struct tm date_and_time;
char* sign;

int internal(const char* path,const struct stat* info, int typeflag, struct FTW* ftwbuf){
    double difference = difftime(info->st_mtime,mktime(&date_and_time));
    if(typeflag != FTW_F){return 0;}
    if(typeflag == FTW_D){return 0;}
    if(typeflag == FTW_SL){return 0;}

    if(*sign == '>' && difference<=0){return 0;}
    if(*sign == '<' && difference>=0){return 0;}
    if(*sign == '=' && difference!=0){return 0;}
    //tutaj juz wiemy ze mamy same interesujace nas pliki
    printf("%s\n",path);
    printf("File size: %ld\n",info->st_size);
    printf("Permissions:\t");
    printf((S_ISDIR(info->st_mode)) ? "d" : "-");
    printf((info->st_mode && S_IRUSR) ? "r" : "-");
    printf((info->st_mode && S_IWUSR) ? "w" : "-");
    printf((info->st_mode && S_IXUSR) ? "x" : "-");
    printf((info->st_mode && S_IRGRP) ? "r" : "-");
    printf((info->st_mode && S_IWGRP) ? "w" : "-");
    printf((info->st_mode && S_IXGRP) ? "x" : "-");
    printf((info->st_mode && S_IROTH) ? "r" : "-");
    printf((info->st_mode && S_IWOTH) ? "w" : "-");
    printf((info->st_mode && S_IXOTH) ? "x" : "-");
    struct tm* mdate;
    mdate = localtime(&info->st_mtime);
    char s[30];
    strftime(s,30,"%d/%m/%Y-%H:%M:%S",mdate);
    printf("\nLast modification date: %s\n\n",s);
    return 0;

}



int main(int argc, char* argv[]) {
    if(argc!=4){printf("use: ./prog ~/xxx < 'DD/MM/YYYY-HH:MM:SS'\n"); return 1;}
    char *path = argv[1];
    char *given_time = argv[3];
    sign = argv[2];
    int flags = FTW_PHYS|FTW_DEPTH;

    printf("Path to root: %s\n",path);
    printf("Date and time of last modification set: %s, sign: %s\n\n", given_time, sign);

    strptime(given_time,"%d/%m/%Y-%H:%M:%S",&date_and_time);

    nftw(path,internal,50,flags);
    return 0;
}