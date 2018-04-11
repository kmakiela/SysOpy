#include <stdlib.h>

int main() {
    int *tab = malloc(sizeof(int) * 1000000000);
    for (int i = 0; i< 1000000000; i++) {
        tab[i] = 9;
    }
    return 0;
}
