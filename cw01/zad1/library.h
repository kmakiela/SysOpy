#ifndef mylibrary
#define mylibrary

char static_table[100][10];

char** create_table_dynamic(int rozmiar_t);

void remove_table(char** table);

void add_block_static(char table[100][10], int index, char block[]);

void remove_block_static(char table[100][10], int index);

char** add_block_dynamic(char** table, int tsize, int index, char block[], int blocksize);

char ** remove_block_dynamic (char** table,int size, int index);

int search_static(char table[100][10],int sum);

int search_dynamic(char** table, int tsize, int bsize, int sum);


#endif
