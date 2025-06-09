#include <stdio.h>
#include <stdlib.h>
#include <time.h>


char gen_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";


int main(int argc, char *argv[]) {
    long key = strtol(argv[1], NULL, 10);
    srand(time(NULL));
    for (int i = 0; i < 256; i++) {
        rand();
    }

}