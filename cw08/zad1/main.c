#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#define  MAX(a, b) ((a > b) ? a : b)
#define  MIN(a, b) ((a < b) ? a : b)

#define RED     "\x1b[91m"
#define BLU     "\x1b[34m"
#define GREEN   "\x1b[92m"
#define DEFAULT "\x1b[0m"
#define YELLOW  "\x1b[33m"

FILE *in_file, *out_file, *filter_file;
int thread_count, width, height, filter_size;
unsigned char** image = NULL;
float** filter = NULL;
unsigned char** out = NULL;

void read_pgm();
void read_filter();
void* use_filter_on_image(void *number);
void write_to_output(unsigned char **im, int width, int height);
void print_answer(const char *name, struct timespec *time_begin, struct timespec *time_end);
void err(const char* msg);
void myexit();

int main(int argc, char const *argv[])
{
    if (argc != 5) err("Wrong arguments");
    thread_count = atoi(argv[1]);
    if (thread_count <= 0) err("Number of threads must be positive");
    if ((in_file = fopen(argv[2], "r")) < 0) err("In");
    if ((filter_file = fopen(argv[3], "r")) < 0) err("Filter");
    if ((out_file = fopen(argv[4], "w")) < 0) err("Out");
    atexit(myexit);
    read_pgm();
    read_filter();

    int i;
    out = (unsigned char**) malloc(height * sizeof(unsigned char*));
    for (i = 0; i<height; i++) out[i] = (unsigned char*) malloc(width * sizeof(unsigned char));
    int *num;
    struct timespec time_begin, time_end;
    pthread_t* thread_table = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
    clock_gettime(CLOCK_MONOTONIC, &time_begin);
    for (i = 0; i < thread_count; i++){
        num = (int*) malloc(sizeof(int));
        *num = i;
        if (pthread_create(&thread_table[i], NULL, &use_filter_on_image, (void*)num)) err("Thread create");
    }
    for (i = 0; i<thread_count; i++) if (pthread_join(thread_table[i], NULL)) err("Thread join");
    clock_gettime(CLOCK_MONOTONIC, &time_end);
    free(thread_table);

    print_answer(argv[2], &time_begin, &time_end);
    write_to_output(out, width, height);

    exit(EXIT_SUCCESS);
}

void read_pgm(){
    char *buf;
    size_t n = 0;
    ssize_t count;
    int offset = 0, i = 0, j = 0, flag = 0;

    getline(&buf, &n, in_file);
    getline(&buf, &n, in_file);
    width = atoi(buf);
    while(!isspace(buf[offset])) offset++;
    height = atoi(buf + offset + 1);
    getline(&buf, &n, in_file);
    image = (unsigned char**) malloc(height * sizeof(unsigned char*));
    while ((count = getline(&buf, &n, in_file)) > 0){
        for (offset = 0; offset < count-1;){
            if (j == 0) image[i] = (unsigned char*) malloc(width * sizeof(unsigned char));
            image[i][j++] = (unsigned char) atoi(buf + offset);
            if (j == width){
                j = 0; 
                i++; 
                if (i == height) {flag = 1; break;}
            }
            while (!isspace(buf[offset]) && offset < count) offset++;
            while (isspace(buf[offset]) && offset < count) offset++;
        }
        if (flag) break;
    }
    free(buf);
}

void read_filter(){
    char *buf;
    size_t n = 0;
    ssize_t count;
    int offset = 0, i = 0, j = 0;
    getline(&buf, &n, filter_file);
    filter_size = atoi(buf);
    filter = (float**) malloc(filter_size * sizeof(float*));

    while ((count = getline(&buf, &n, filter_file)) > 0){
        for (offset = 0; offset < count-1;){
            if (j == 0) filter[i] = (float*) malloc(filter_size * sizeof(float));
            filter[i][j++] = atof(buf + offset);
            if (j == filter_size) {j = 0; i++;}
            while (!isspace(buf[offset]) && offset < count) offset++;
            while (isspace(buf[offset]) && offset < count) offset++;
        }
    }
    free(buf);
}

void* use_filter_on_image(void *number){
    int i = height*(*(int*)number) / thread_count;
    int end = height*(*(int*)number + 1) / thread_count;
    int j, k, l;
    int c2 = ceil(((float)filter_size)/2.0);
    float s;
    free(number);
    for (; i < end; i++){
        for (j = 0; j < width; j++){
            s = 0;
            for (k = 0; k < filter_size; k++) for (l = 0; l < filter_size; l++)
                s += filter[k][l] * image[MIN(height-1,MAX(0, i-c2+k))][MIN(width-1, MAX(0, j-c2+l))];
            out[i][j] = round(s);
        }
    }
    pthread_exit((void*) 0);
}

void write_to_output(unsigned char **im, int width, int height){
    int i, j, k = 0;
    char buf[100];
    sprintf(buf, "P2\n%i %i\n255\n", width, height);
    fwrite(buf, 1, (size_t) strlen(buf), out_file);
    for (i = 0; i<height; i++) for (j = 0; j<width; j++){
        if (k == 0) buf[0] = '\0';
        sprintf(buf, "%s%i", buf, im[i][j]);
        if (k == 20){
            sprintf(buf, "%s\n", buf);
            fwrite(buf, 1, (size_t) strlen(buf), out_file);
            k = 0;
        }
        else{
            sprintf(buf, "%s ", buf);
            k++;
        }
    }
    if (k != 0){
        sprintf(buf, "%s\n", buf);
        fwrite(buf, 1, (size_t) strlen(buf), out_file);
    }
}

void print_answer(const char *name, struct timespec *time_begin, struct timespec *time_end){
    time_t s = time_end->tv_sec - time_begin->tv_sec;
    long n = time_end->tv_nsec - time_begin->tv_nsec;
    if (n < 0){
        s--;
        n += 1000000000;
    }
    printf(YELLOW"\n%d "DEFAULT"Thread(s) with filter size of "YELLOW"%d "DEFAULT"filtered file "BLU"%s "DEFAULT"in "GREEN"%ld:%ld\n"DEFAULT,thread_count,filter_size,name,s,n);
}

void err(const char* msg){
    if (errno) perror(msg);
    else printf(RED"%s\n"DEFAULT, msg);
    exit(EXIT_FAILURE);
}

void myexit() {
    fclose(in_file);
    fclose(filter_file);
    fclose(out_file);
    int i;
    if(image != NULL){
        for(i=0;i<height;i++) free(image[i]);
        free(image);
    }
    if(filter != NULL){
        for(i=0;i<filter_size;i++) free(filter[i]);
        free(filter);
    }
    if(out != NULL){
        for(i=0;i<height;i++) free(out[i]);
        free(out);
    }
}
