#include "library.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char** create_table_dynamic(int rozmiar_t){
    return malloc(rozmiar_t*sizeof(char*));
}

void remove_table(char** table) {
    int i;
    for(i=0;i<sizeof(table)/sizeof(table[0]);i++){
        free(table[i]);
    }
    free(table);
}

void add_block_static(char table[100][10], int index, char block[10]){
    int i;
    int j;
    for(j=99;j>index;j--){
        for(i=0;i<10;i++){
            table[j][i] = table[j-1][i]; //tracimy zawartość ostatniego bloku
        }
    }
    for(i=0;i<10;i++){
        table[index][i] = block[i];
    }
}

void remove_block_static(char table[100][10], int index){
    int i;
    int j;
    for(j=index;j<99;j++) {
        for (i = 0; i < 10; i++) {
            table[j][i] = table[j+1][i];
        }
    }
}

char** add_block_dynamic(char** table, int tsize, int index, char block[], int blocksize){
    if(index >=tsize || index<0){printf("Index nieprawidlowy"); return table;}
    table = realloc(table,((tsize*sizeof(char*)+sizeof(char*))));
    table[tsize] = strcpy(malloc(blocksize*sizeof(char)), block);
    int pos = tsize;
    while(pos!=index){
        char* tmp = table[pos-1];
        table[pos-1] = table[pos];
        table[pos] = tmp;
        pos--;
    }
    return table;
}

char** remove_block_dynamic(char** table, int tsize, int index){
    int j;
    free(table[index]);
    for(j=index;j<tsize-1;j++){
        table[j] = table[j+1];
    }
    table = realloc(table, (tsize-1)*sizeof(char*));
}

int search_static(char table[100][10],int sum){
    int diff = sum;
    int result;
    int i;
    int j;
    for(i=0;i<100;i++){
        int sum_tmp = 0;
        for(j=0;j<100;j++){
            sum_tmp += (int) static_table[i][j];
        }
        if(sum-sum_tmp<diff) {result = i; diff = sum-sum_tmp;}
    }
    return result;
}

int search_dynamic(char** table,int tsize, int bsize, int sum){
    int diff = sum;
    int i;
    int j;
    int pos;
    for(i=0;i<tsize;i++){

        int sum_tmp = 0;
        if(table[i]!=NULL){
            for(j=0;j<bsize;j++){
                sum_tmp += (int) table[i][j];
            }
            //printf("suma %d bloku to %d\n",i,sum_tmp);
            if(abs(sum - sum_tmp)<diff) {
                //printf("znaleziona suma mniejsza od dotychczasowej dla indexu %d\n",i);
                pos = i;
                diff = abs(sum-sum_tmp);
            }
        }
    }
    return pos;
}
