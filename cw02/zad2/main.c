#define _XOPEN_SOURCE 700
#include <time.h>
#include <sys/times.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
void search(char* path, char* sign, struct tm date){
    struct stat info;
    struct dirent* file;
    char* filename;
    char filepath[1000];

    DIR* root = opendir(path);
    if(!root){printf("Nie da się otworzyć katalogu"); return;}
    file = readdir(root);
    while(file!=NULL){
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0){
            file = readdir(root);
            continue;
        }
        filename=file->d_name;
        snprintf(filepath,1000,"%s/%s", path, filename);
        lstat(filepath,&info);

        if(S_ISDIR(info.st_mode)){
            search(filepath,sign,date);
            file = readdir(root);
            continue;
        }
        if(!S_ISREG(info.st_mode)){file = readdir(root);continue;}
        if(S_ISBLK(info.st_mode)){file = readdir(root);continue;}
        if(S_ISLNK(info.st_mode)){file = readdir(root);continue;}
        double difference = difftime(mktime(&date),info.st_mtime);
        if(*sign == '>' && difference>=0){file = readdir(root);continue;}
        if(*sign == '<' && difference<=0){file = readdir(root);continue;}
        if(*sign == '=' && difference!=0){file = readdir(root);continue;}
        //tutaj juz wiemy ze mamy same interesujace nas pliki
        printf("%s\n",filepath);
        printf("File size: %ld\n",info.st_size);
        printf("Permissions:\t");
        printf((S_ISDIR(info.st_mode)) ? "d" : "-");
        printf((info.st_mode && S_IRUSR) ? "r" : "-");
        printf((info.st_mode && S_IWUSR) ? "w" : "-");
        printf((info.st_mode && S_IXUSR) ? "x" : "-");
        printf((info.st_mode && S_IRGRP) ? "r" : "-");
        printf((info.st_mode && S_IWGRP) ? "w" : "-");
        printf((info.st_mode && S_IXGRP) ? "x" : "-");
        printf((info.st_mode && S_IROTH) ? "r" : "-");
        printf((info.st_mode && S_IWOTH) ? "w" : "-");
        printf((info.st_mode && S_IXOTH) ? "x" : "-");
        struct tm* mdate;
        mdate = localtime(&info.st_mtime);
        char s[30];
        strftime(s,30,"%d/%m/%Y-%H:%M:%S",mdate);
        printf("\nLast modification date: %s\n\n",s);
        file = readdir(root);
    }
    closedir(root);
}



int main(int argc, char* argv[]) {
    if(argc!=4){printf("use: ./prog ~/xxx < 'DD/MM/YYYY-HH:MM:SS'\n"); return 1;}
    char *path = argv[1];
    char *sign = argv[2];
    char *given_time = argv[3];

    printf("Path to root: %s\n",path);
    printf("Date and time of last modification set: %s, sign: %s\n\n", given_time, sign);

    struct tm date_and_time;
    strptime(given_time,"%d/%m/%Y-%H:%M:%S",&date_and_time);
    search(path, sign, date_and_time);
}