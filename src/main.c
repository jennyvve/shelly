
#include "stdio.h"
#include "stdlib.h"

int main(int argc, int**) {
    if (argc != 0) {
        printf("Shelly does not support arguments.");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
