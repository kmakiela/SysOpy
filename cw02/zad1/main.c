#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

void generate(char* file_name, int record_no, long int record_size){
    int handle;
    handle = open(file_name,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    int i,j;
    char* bufor = malloc((record_size)*sizeof(char));
    for(i=0;i<record_no;i++){
        for(j=0;j<record_size-1;j++){
            bufor[j] = 'a' + (rand()%26);
        }
        bufor[record_size-1] = '\n';
        write(handle,bufor,record_size);
    }
    close(handle);
    free(bufor);
}

void sort_sys(char* file_name, long int record_size){
    int handle = open(file_name,O_RDWR, S_IRUSR|S_IWUSR);
    long int file_end = lseek(handle,0,SEEK_END);
    lseek(handle,0,SEEK_SET);
    int swap = 1;
    while(swap){
        lseek(handle,0, SEEK_SET);
        swap = 0;
        char* r1 = malloc(record_size*sizeof(char));
        char* r2 = malloc(record_size*sizeof(char));

        if(lseek(handle,0,SEEK_CUR)<file_end){read(handle,r1,record_size);}
        while(lseek(handle,0,SEEK_CUR)<file_end){
            read(handle,r2,record_size);
            if(r1[0]<=r2[0]){
                char * tmp = r1;
                r1 = r2;
                r2 = tmp;
            }
            else{
                lseek(handle,-(2*record_size),SEEK_CUR);
                write(handle,r2,record_size);
                write(handle,r1,record_size);
                swap = 1;
            }
        }
        free(r1);
        free(r2);
    }
    close(handle);
}

void copy_sys(char * in_name,char* out_name,long int record_size){
    int in = open(in_name, O_RDWR,S_IRUSR|S_IWUSR);
    int out = open(out_name,O_RDWR|O_CREAT,S_IRUSR|S_IWUSR);
    lseek(in,0,SEEK_END);
    long int in_end = lseek(in,0,SEEK_CUR);
    lseek(in,0,SEEK_SET);
    lseek(out,0,SEEK_SET);
    char* bufor = malloc(record_size*sizeof(char));
    while(lseek(in,0,SEEK_CUR)<in_end){
        read(in,bufor,record_size);
        write(out,bufor,record_size);
    }
    close(in);
    close(out);
    free(bufor);
}

void sort(char* file_name, long int record_size){
    FILE* handle = fopen(file_name,"r+");
    fseek(handle,0,SEEK_END);
    long int file_end = ftell(handle);
    fseek(handle,0,SEEK_SET);
    int swap = 1;
    while(swap){
        fseek(handle,0, SEEK_SET);
        swap = 0;
        char* r1 = malloc(record_size*sizeof(char));
        char* r2 = malloc(record_size*sizeof(char));
        if(ftell(handle)<file_end){fread(r1,1,record_size,handle);}
        while(ftell(handle)<file_end){
            fread(r2,1,record_size,handle);
            if(r1[0]<=r2[0]){
                char * tmp = r1;
                r1 = r2;
                r2 = tmp;
            }
            else{
                fseek(handle,-(2*record_size),SEEK_CUR);
                fwrite(r2,1,record_size,handle);
                fwrite(r1,1,record_size,handle);
                swap = 1;
            }
        }
        free(r1);
        free(r2);
    }
    fclose(handle);
}

void copy(char* in_name, char* out_name,long int record_size){
    FILE* in = fopen(in_name,"r");
    FILE* out = fopen(out_name,"w");
    fseek(in,0,SEEK_END);
    long int in_end = ftell(in);
    fseek(in,0,SEEK_SET);
    fseek(out,0,SEEK_SET);
    char* bufor = malloc(record_size*sizeof(char));
    while(ftell(in)<in_end){
        fread(bufor,1,record_size,in);
        fwrite(bufor,1,record_size,out);
    }
    free(bufor);
    fclose(in);
    fclose(out);
}

void parser(int argc, char* argv[]){
    if(strcmp(argv[1],"generate")==0){
            if(argc!=5){printf("./program generate name record_no record_size\n");}
            int record_no, record_size;
            if(sscanf(argv[3],"%d",&record_no)!=1){printf("Number of records must be an Integer\n");}
            if(sscanf(argv[4],"%d",&record_size)!=1){printf("Record's size must be an Integer\n");}
            generate(argv[2],record_no,record_size);
        }
    if(strcmp(argv[1],"sort")==0){
            if(argc!=6){printf("./program sort name record_no record_size variant\n");}
            int record_size;
            if(sscanf(argv[4],"%d",&record_size)!=1){printf("Record's size must be an Integer\n");}
            if(strcmp(argv[5],"sys")==0) {sort_sys(argv[2],record_size);}
            if(strcmp(argv[5],"lib")==0) {sort(argv[2],record_size);}
        }
    if(strcmp(argv[1],"copy")==0){
            if(argc!=7){printf("./program copy name name2 record_no record_size variant\n");}
            int record_size;
            if(sscanf(argv[5],"%d",&record_size)!=1){printf("Record's size must be an Integer\n");}
            if(strcmp(argv[6],"sys")==0) {copy_sys(argv[2],argv[3],record_size);}
            if(strcmp(argv[6],"lib")==0) {copy(argv[2],argv[3],record_size);}
        }
}

void czas_sortowania(char* name, int size,int count){
    FILE *fp;
    fp = fopen("wyniki.txt","at");
    struct rusage ru,ru2;
    struct timeval tv,tv2;
    struct timeval utime,utime2;
    struct timeval stime,stime2;
    struct timezone tz,tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    sort(name,size);
    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec;if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    fprintf(fp,"Czas sortowania %d rekordów %d-bajtowych z funkcjami bibliotecznymi:\nRealtime: %ld.%lds\nUsertime: %ld.%lds\nSystemtime: %ld.%lds\n",count,size,rs,rms,us,ums,ss,sms);
    fclose(fp);
}

void czas_kopiowania_sys(char* name,char* out, int size,int count){
    FILE *fp;
    fp = fopen("wyniki.txt","at");
    struct rusage ru,ru2;
    struct timeval tv,tv2;
    struct timeval utime,utime2;
    struct timeval stime,stime2;
    struct timezone tz,tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;

    copy_sys(name,out,size);

    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec;if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    fprintf(fp,"Czas kopiowania systemowego %d rekordów %d-bajtowych:\nRealtime: %ld.%lds\nUsertime: %ld.%lds\nSystemtime: %ld.%lds\n",count,size,rs,rms,us,ums,ss,sms);
    fclose(fp);
}

void czas_kopiowania(char* name,char* out, int size,int count) {
    FILE *fp;
    fp = fopen("wyniki.txt", "at");
    struct rusage ru, ru2;
    struct timeval tv, tv2;
    struct timeval utime, utime2;
    struct timeval stime, stime2;
    struct timezone tz, tz2;
    gettimeofday(&tv, &tz);
    getrusage(RUSAGE_SELF, &ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;

    copy(name, out, size);

    gettimeofday(&tv2, &tz2);
    getrusage(RUSAGE_SELF, &ru2);
    utime2 = ru2.ru_utime;
    stime2 = ru2.ru_stime;
    long int rs, rms, us, ums, ss, sms;
    rs = tv2.tv_sec - tv.tv_sec;
    rms = tv2.tv_usec - tv.tv_usec;
    if (rms < 0) {rs--;rms = 1000000 + rms;}
    us = utime2.tv_sec - utime.tv_sec;
    ums = utime2.tv_usec - utime.tv_usec;
    if (ums < 0) {us--;ums = 1000000 + ums;}
    ss = stime2.tv_sec - stime.tv_sec;
    sms = stime2.tv_usec - stime.tv_usec;
    if (sms < 0) {ss--;sms = 1000000 + sms;}
    fprintf(fp,"Czas kopiowania %d rekordów %d-bajtowych z wykorzystaniem funkcji bibliotecznych:\nRealtime: %ld.%lds\nUsertime: %ld.%lds\nSystemtime: %ld.%lds\n", count, size, rs, rms, us, ums, ss, sms);
    fclose(fp);
}

    void czas_sortowania_sys(char* name, int size,int count){
    FILE *fp;
    fp = fopen("wyniki.txt","at");
    struct rusage ru,ru2;
    struct timeval tv,tv2;
    struct timeval utime,utime2;
    struct timeval stime,stime2;
    struct timezone tz,tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;

    sort_sys(name,size);

    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec;if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    fprintf(fp,"Czas sortowania systemowego %d rekordów %d-bajtowych:\nRealtime: %ld.%lds\nUsertime: %ld.%lds\nSystemtime: %ld.%lds\n",count,size,rs,rms,us,ums,ss,sms);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        parser(argc, argv);
    }
    else {
        char *s4 = "test4";
        char* s42 = "test4v2";
        char* s4c = "test4_copy";
        char* s42c = "test4v2_copy";
        char* s5 = "test512";
        char* s52 = "test512v2";
        char* s5c = "test512_copy";
        char* s52c = "test512v2_copy";
        char* s4k = "test4k";
        char* s4k2 = "test4kv2";
        char* s4kc = "test4k_copy";
        char* s4k2c = "test4kv2_copy";
        char* s8 = "test8";
        char* s82 = "test8v2";
        char* s8c = "test8_copy";
        char* s82c = "test8v2_copy";
        generate(s4,3000,4);
        copy(s4,s42,4);
        czas_sortowania_sys(s4,4,3000);
        czas_sortowania(s42,4,3000);
        czas_kopiowania(s42,s42c,4,3000);
        czas_kopiowania_sys(s4,s4c,4,3000);
        generate(s4,4000,4);
        copy_sys(s4,s42,4);
        czas_sortowania_sys(s4,4,4000);
        czas_sortowania(s42,4,4000);
        czas_kopiowania(s42,s42c,4,4000);
        czas_kopiowania_sys(s4,s4c,4,4000);
        //testy dla 512bajtowych
        generate(s5,2000,512);
        copy(s5,s52,512);
        czas_sortowania_sys(s5,512,2000);
        czas_sortowania(s52,512,2000);
        czas_kopiowania(s52,s52c,512,2000);
        czas_kopiowania_sys(s5,s52,512,2000);
        generate(s5,1000,512);
        copy_sys(s5,s52,512);
        czas_sortowania_sys(s5,512,1000);
        czas_sortowania(s52,512,1000);
        czas_kopiowania(s52,s52c,512,1000);
        czas_kopiowania_sys(s5,s5c,512,1000);
        //testy dla 4096bajtowych
        generate(s4k, 1000, 4096);
        copy(s4k, s4k2, 4096);
        //czas_sortowania_sys(s4k, 4096, 10);
        //czas_sortowania(s4k2, 4096, 10);
        czas_kopiowania(s4k2, s4k2c, 4096, 1000);
        czas_kopiowania_sys(s4k, s4kc, 4096, 1000);
        generate(s4k, 1500, 4096);
        copy_sys(s4k,s4k2, 4096);
        //czas_sortowania_sys(s4k, 4096, 20);
        //czas_sortowania(s4k2,4096, 20);
        czas_kopiowania(s4k2, s4k2c, 4096, 1500);
        czas_kopiowania_sys(s4k, s4kc, 4096, 1500);
        //testy dla 8192 bajtowych
        generate(s8, 1500, 8192);
        copy(s8,s82, 8192);
        czas_sortowania_sys(s8, 8192, 1500);
        czas_sortowania(s82, 8192, 1500);
        czas_kopiowania(s82, s82c, 8192, 1500);
        czas_kopiowania_sys(s8, s8c, 8192, 1500);
        generate(s8, 1000, 8192);
        copy_sys(s8, s82, 8192);
        czas_sortowania_sys(s8, 8192, 1000);
        czas_sortowania(s82, 8192, 1000);
        czas_kopiowania(s82, s82c, 8192, 1000);
        czas_kopiowania_sys(s8, s8c, 8192, 1000);

    }
    return 0;
}