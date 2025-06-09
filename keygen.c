#include <stdio.h>
#include <stdlib.h>
#include <time.h>


char gen_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";


int main(int argc, char *argv[]) {
    long key = strtol(argv[1], NULL, 10);
    srand(time(NULL));
    for (int i = 0; i < key; i++) {
        int rand_num = rand() % 27;
        putchar(gen_chars[rand_num]);
    }
    putchar('\n');
    return 0;
}
