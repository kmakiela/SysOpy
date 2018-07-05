//
// Created by kmakiela on 24.05.18.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

int main(int argc, char* argv[]){
    if(argc != 3) printf("use ./gen name N\n");
    char* name = argv[1];
    int size = atoi(argv[2]);
    FILE* file;
    if ((file = fopen(name, "w")) < 0){printf("File error"); return 1;}
    int* tab = malloc(size*size*sizeof(int));
    srand(time(NULL));
    long long int sum = 0;
    int i,j;
    for(i=0;i<size;i++){
        for(j=0;j<size;j++){
            tab[i*size+j] = rand();
            sum+=tab[i*size+j];
        }
    }
    float* float_tab = malloc(size*size*sizeof(float));
    for(i=0;i<size;i++){
        for(j=0;j<size;j++){
            float_tab[i*size+j] = (float)tab[i*size+j] / sum;
        }
    }
    char buf[30];
    sprintf(buf, "%d\n", size);
    fwrite(buf, 1, (size_t) strlen(buf), file);
    for(i=0;i<size;i++){
        for(j=0;j<size;j++){
            sprintf(buf,"%.10f\n",float_tab[i*size+j]);
            fwrite(buf,1,(size_t) strlen(buf),file);
        }
    }
}
