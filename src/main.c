
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "exec.h"
#include "generic.h"
#include "parse.h"
#include "token.h"

sy_rt_e sy_terminal_start(void) {
    size_t path_size = pathconf(".", _PC_PATH_MAX);
    char *path;
    char *path_ptr;

    if ((path = malloc(sizeof(path) * path_size)) == NULL) {
        printf(
            "ERR! internal error, unable to allocate path memory.");
        return SY_RT_ERR;
    }

    if ((path_ptr = getcwd(path, path_size)) == NULL) {
        printf(
            "ERR! internal error, failed to retrieve the current "
            "path.");
        return SY_RT_ERR;
    }

    sy_token_man_t *man;
    if ((man = sy_token_man_malloc()) == NULL) {
        printf("ERR! internal error, failed to allocate manager.\n");
        return SY_RT_ERR;
    }

    char input[SY_MAX_ARG_LENGTH];
    char *str;

    while (true) {
        sy_rt_e rt = SY_RT_ERR;

        printf("%s$ ", path_ptr);
        fflush(stdout);

        sy_token_node_t *node;
        if ((node = sy_token_new(man)) == NULL) {
            printf("ERR! internal error, failed to allocate node.\n");
            return SY_RT_ERR;
        }

        while (
            (str = fgets(input, SY_MAX_ARG_LENGTH, stdin)) != NULL &&
            (rt = sy_parse(man, node, str) != SY_RT_FOUND_NEWLINE) &&
            rt != SY_RT_ERR);

        if (rt != SY_RT_ERR) {
            if ((rt = sy_exec(path_ptr, node)) == SY_RT_ERR) {
                printf("ERR! failed to execute command.");
            }

            if (rt == SY_RT_EXIT) {
                return SY_RT_OK;
            }
        } else {
            printf("ERR! failed to parse command.");
        }

        sy_token_man_clear(man);
    }
}

int main(void) {
    if (sy_terminal_start() != SY_RT_OK) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}