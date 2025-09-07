#include <stdio.h>
#include <stdlib.h>

static int window_closed = 0;

void cleanup(void) {
    // if (!window_closed)
    // window_closed = 1;
}

int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++)
        printf("arg: %s\n", argv[i]);
    printf("asdasqd  \n");

    atexit(&cleanup);

    // CloseWindow();

    return 0;
}
