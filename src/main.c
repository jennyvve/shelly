/**
 * @file main.c
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com,s3787494)
 * @brief Simple shell.
 *
 * @copyright Copyright (c) 2026
 */

#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "generic.h"
#include "interp.h"
#include "lexer.h"
#include "token.h"

sy_rt_e sy_terminal_start(char *path, sy_token_man_t *man) {
    char input_buffer[SY_MAX_ARG_LENGTH];
    char *input_ptr;
    sy_rt_e rt = SY_RT_ERR;

    do {
        printf("%s$ ", path);
        fflush(stdout);

        sy_token_node_t *node;
        if ((node = sy_token_new(man)) == NULL) {
            printf("!ERR internal error, failed to allocate node.\n");
            return SY_RT_ERR;
        }

        while ((input_ptr = fgets(input_buffer, SY_MAX_ARG_LENGTH,
                                  stdin)) != NULL &&
               (rt = sy_lexer(man, node, input_ptr) !=
                     SY_RT_FOUND_NEWLINE) &&
               rt != SY_RT_ERR);

        rt = (rt != SY_RT_ERR) ? sy_interp(path, node) : rt;

        if (rt == SY_RT_ERR) {
            printf("!ERR failed to interpret command.\n");
        }

        sy_token_man_clear(man);
    } while (rt != SY_RT_EXIT);

    return SY_RT_OK;
}

int main(void) {
    size_t path_size = pathconf(".", _PC_PATH_MAX);
    char *path;
    char *path_ptr;

    if ((path = malloc(path_size)) == NULL) {
        printf(
            "!ERR internal error, unable to allocate path "
            "memory.");
        return SY_RT_ERR;
    }

    if ((path_ptr = getcwd(path, path_size)) == NULL) {
        printf(
            "!ERR internal error, failed to retrieve the current "
            "path.");
        return SY_RT_ERR;
    }

    sy_token_man_t *man;
    if ((man = sy_token_man_malloc()) == NULL) {
        printf("!ERR internal error, failed to allocate manager.\n");
        return EXIT_FAILURE;
    }

    signal(SIGINT, SIG_IGN);

    if (sy_terminal_start(path_ptr, man) != SY_RT_OK) {
        return EXIT_FAILURE;
    }

    sy_token_man_free(man);
    free(path);

    return EXIT_SUCCESS;
}