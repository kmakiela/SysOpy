#include <stdio.h>
#include <stdlib.h>
#include "library.h"
#include <string.h>
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

void parser(int argc, char* argv[]) {
    //printf("liczba argumentów to %d\n", argc-1);
    int i;
    //for (i = 0; i < argc; i++) { printf("arg no%d to %s\n", i, argv[i]); }
    char **p;
    int t, b;
    for (i = 0; i < argc; i++) {
        if (strcmp(argv[i], "create_table") == 0) {
            int tsize = charToInt(argv[i + 1]);
            t = tsize;
            int bsize = charToInt(argv[i + 2]);
            b = bsize;
            p = create_table_dynamic(0);
            int j;
            for (j = 0; j < tsize; j++) {
                char s[bsize];
                int k;
                for (k = 0; k < bsize; k++) {
                    s[k] = (j * k*k*k) % 200;
                }
                p = add_block_dynamic(p, tsize, 0, s, bsize);
            }
            printf("Alokacja %d elementowej tablicy o długościach bloków: %d\n", tsize, bsize);
        }
        if (strcmp(argv[i], "search_element") == 0) {
            int wartosc = charToInt(argv[i + 1]);
            int wynik;
            wynik = search_dynamic(p, t, b, wartosc);
            printf("Index szukanego bloku to:%d\n", wynik);
        }
        if (strcmp(argv[i], "remove") == 0) {
            int pc = charToInt(argv[i + 1]);
            int i;
            for (i = 0; i < pc; i++) {
                p = remove_block_dynamic(p, t, t - 1);
                t--;
            }
            printf("usunięto %d bloków\n", pc);
        }
        if (strcmp(argv[i], "add") == 0) {
            int pc = charToInt(argv[i + 1]);
            int i;
            for (i = 0; i < pc; i++) {
                char s[b];
                int j;
                for (j = 0; j < b; j++) {
                    s[j] = (char) (i * j) % 100 + 50;
                }
                p = add_block_dynamic(p, t, t - 1, s, b);
                t++;
            }
            printf("Dodano %d bloków\n", pc);
        }
        if (strcmp(argv[i], "remove_and_add") == 0) {
            int pc = charToInt(argv[i + 1]);
            int i;
            for (i = 0; i < pc; i++) {
                char s[b];
                int j;
                for (j = 0; j < b; j++) {
                    s[j] = (char) (i * j) % 100 + 50;
                }
                p = add_block_dynamic(p, t, t - 1, s, b);
                t++;

                p = remove_block_dynamic(p, t, t - 1);
                t--;

            }
            printf("dodano i usunięto %d bloków\n", pc);
        }
    }
}

void czas_tworzenia(){
    FILE *fp;
    fp = fopen("raport2.txt","at");
    struct rusage ru;
    struct rusage ru2;
    struct timeval tv;
    struct timeval tv2;
    struct timeval utime;
    struct timeval utime2;
    struct timeval stime;
    struct timeval stime2;
    struct timezone tz;
    struct timezone tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    //printf("Realtime:\nseconds: %ld, microseconds: %ld\nUser Time:\n seconds: %ld, microseconds:%ld\nSystem Time:\n seconds:%ld, microseconds:%ld\n",tv.tv_sec,tv.tv_usec,utime.tv_sec,utime.tv_usec,stime.tv_sec,stime.tv_usec);
    //fprintf(fp,"Realtime:\nseconds: %ld, microseconds: %ld\nUser Time:\n seconds: %ld, microseconds:%ld\nSystem Time:\n seconds:%ld, microseconds:%ld\n",tv.tv_sec,tv.tv_usec,utime.tv_sec,utime.tv_usec,stime.tv_sec,stime.tv_usec);
    int i, timer = 50000;
    for(i=0;i<timer;i++){
        int tsize = 100;
        int bsize = 10;
        char** p = create_table_dynamic(0);
        int j;
        for (j = 0; j < tsize; j++) {
            char s[bsize];
            int k;
            for (k = 0; k < bsize; k++) {
                s[k] = (j * k*k*k) % 200;
            }
            p = add_block_dynamic(p, tsize, 0, s, bsize);
        }
    }
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
    printf("Całowity czas 50 000 operacji dodawania nowej tabeli o rozmiarze 100 i o długości bloku równej 10 to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fprintf(fp,"Całowity czas 50 000 operacji dodawania nowej tabeli o rozmiarze 100 i o długości bloku równej 10 to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fclose(fp);

}

void czas_szukania() {
    int t = 1000;
    int b = 5;
    char** p = create_table_dynamic(0);
    int j;
    for (j = 0; j < t; j++) {
        char s[b];
        int k;
        for (k = 0; k < b; k++) {
            s[k] = (char) (j * k * k * k) % 200;
        }
        p = add_block_dynamic(p, t, 0, s, b);
    }

    FILE *fp;
    fp = fopen("raport2.txt","at");
    struct rusage ru;
    struct rusage ru2;
    struct timeval tv;
    struct timeval tv2;
    struct timeval utime;
    struct timeval utime2;
    struct timeval stime;
    struct timeval stime2;
    struct timezone tz;
    struct timezone tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    for (j = 0; j < 50000; j++) {
        search_dynamic(p, t, b, j);
    }
    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec; if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    printf("Całowity czas 50 000 operacji wyszukiwania to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fprintf(fp,"\nCałowity czas 50 000 operacji wyszukiwania to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fclose(fp);
    free(p);

}

void czas_dodania_i_usuwania() {
    int t = 1000;
    int b = 5;
    char** p = create_table_dynamic(0);
    int j;
    for (j = 0; j < t; j++) {
        char s[b];
        int k;
        for (k = 0; k < b; k++) {
            s[k] = (char) (j * k * k * k) % 200;
        }
        p = add_block_dynamic(p, t, 0, s, b);
    }

    FILE *fp;
    fp = fopen("raport2.txt","at");
    struct rusage ru;
    struct rusage ru2;
    struct timeval tv;
    struct timeval tv2;
    struct timeval utime;
    struct timeval utime2;
    struct timeval stime;
    struct timeval stime2;
    struct timezone tz;
    struct timezone tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    for (j = 0; j < 50000; j++) {
       add_block_dynamic(p,1000,0,p[1],5);
        remove_block_dynamic(p,1001,0);
    }
    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec; if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    printf("Całowity czas 50 000 operacji dodania i usunięcia bloku to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fprintf(fp,"\nCałowity czas 50 000 operacji dodania i usunięcia bloku to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fclose(fp);
    free(p);

}

void czas_naprzemiennego_dodania_i_usuwania() {
    int t = 1000;
    int b = 5;
    char** p = create_table_dynamic(0);
    int j;
    for (j = 0; j < t; j++) {
        char s[b];
        int k;
        for (k = 0; k < b; k++) {
            s[k] = (char) (j * k * k * k) % 200;
        }
        p = add_block_dynamic(p, t, 0, s, b);
    }

    FILE *fp;
    fp = fopen("raport2.txt","at");
    struct rusage ru;
    struct rusage ru2;
    struct timeval tv;
    struct timeval tv2;
    struct timeval utime;
    struct timeval utime2;
    struct timeval stime;
    struct timeval stime2;
    struct timezone tz;
    struct timezone tz2;
    gettimeofday(&tv,&tz);
    getrusage(RUSAGE_SELF,&ru);
    utime = ru.ru_utime;
    stime = ru.ru_stime;
    for (j = 0; j < 50000; j++) {
        add_block_dynamic(p,1000,0,p[1],5);
        remove_block_dynamic(p,1001,0);
        add_block_dynamic(p,1000,0,p[1],5);
        remove_block_dynamic(p,1001,0);
        add_block_dynamic(p,1000,0,p[1],5);
        remove_block_dynamic(p,1001,0);
    }
    gettimeofday(&tv2,&tz2);
    getrusage(RUSAGE_SELF,&ru2);
    utime2=ru2.ru_utime;
    stime2=ru2.ru_stime;
    long int rs,rms,us,ums,ss,sms;
    rs = tv2.tv_sec-tv.tv_sec;
    rms = tv2.tv_usec-tv.tv_usec; if(rms<0){rs--;rms = 1000000+rms;}
    us = utime2.tv_sec-utime.tv_sec;
    ums = utime2.tv_usec-utime.tv_usec;if(ums<0){us--;ums = 1000000+ums;}
    ss = stime2.tv_sec-stime.tv_sec;
    sms = stime2.tv_usec-stime.tv_usec;if(sms<0){ss--;sms = 1000000+sms;}
    printf("Całowity czas 50 000 operacji naprzemiennego dodania i usunięcia bloku to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fprintf(fp,"\nCałowity czas 50 000 operacji naprzemiennego dodania i usunięcia bloku to:\nRealtime: %ld sekund i %ld mikrosekund\nUsertime: %ld sekund i %ld mikrosekund\nSystemtime: %ld sekund i %ld mikrosekund\n",rs,rms,us,ums,ss,sms);
    fclose(fp);
    free(p);

}

int main(int argc, char* argv[]) {
    if(argc!=1) {
        parser(argc, argv);
    }
    else {
        czas_tworzenia();
        czas_szukania();
        czas_dodania_i_usuwania();
        czas_naprzemiennego_dodania_i_usuwania();
    }
}