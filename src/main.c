
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "generic.h"

sy_rt_e sy_terminal_start(void) {
    size_t path_size = pathconf(".", _PC_PATH_MAX);
    char *path;
    char *path_ptr;

    if ((path = malloc(sizeof(path) * path_size)) == NULL) {
        printf("ERR! unable to allocate path memory.");
        return SY_RT_ERR;
    }

    if ((path_ptr = getcwd(path, path_size)) == NULL) {
        printf("ERR! failed to retrieve the current path.");
        return SY_RT_ERR;
    }

    printf("%s$ ", path_ptr);

    char input[SY_MAX_ARG_LENGTH];
    scanf("");

    return SY_RT_OK;
}

int main(int argc, int **) {
    if (argc != 0) {
        printf("Shelly does not support arguments.");
        return EXIT_FAILURE;
    }

    if (sy_terminal_start() != SY_RT_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}