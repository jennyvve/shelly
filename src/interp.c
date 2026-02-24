#include "interp.h"

#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cmd.h"

typedef char *interp_argv_t[SY_MAX_PIPEC][SY_MAX_ARGC + 1];

sy_rt_e exec(char *path, char *argv[SY_MAX_ARG_LENGTH],
             unsigned int *pid_index, pid_t pids[SY_MAX_PIDS], int in,
             int out) {
    sy_rt_e rt;
    if ((rt = sy_cmd_run(path, argv)) != SY_RT_NOT_FOUND) {
        return rt;
    }

    pids[*pid_index] = sy_cmd_spawn(path, argv, in, out);
    if (pids[*pid_index] == -1) {
        return SY_RT_ERR;
    }

    (*pid_index)++;
    return SY_RT_OK;
}

typedef sy_rt_e (*interp_t)(char *path, sy_token_node_t *node,
                            unsigned int *pipec, unsigned int *argc,
                            interp_argv_t argv,
                            pid_t pids[SY_MAX_PIDS]);

interp_t interpreter[SY_TOKEN_INVALID + 1];

sy_rt_e interp_command(char *path, sy_token_node_t *node,
                       unsigned int *pipec, unsigned int *argc,
                       interp_argv_t argv, pid_t pids[SY_MAX_PIDS]) {
    if (*argc != 0) {
        return SY_RT_ERR;
    }

    argv[*pipec][(*argc)++] = node->value;
    return SY_RT_OK;
}

sy_rt_e interp_arg(char *path, sy_token_node_t *node,
                   unsigned int *pipec, unsigned int *argc,
                   interp_argv_t argv, pid_t pids[SY_MAX_PIDS]) {
    if (*argc >= SY_MAX_ARGC) {
        return SY_RT_ERR;
    }

    argv[*pipec][(*argc)++] = node->value;
    return SY_RT_OK;
}

sy_rt_e interp_pipeline(char *path, sy_token_node_t *node,
                        unsigned int *pipec, unsigned int *argc,
                        interp_argv_t argv, pid_t pids[SY_MAX_PIDS]) {
    if (*pipec >= SY_MAX_PIPEC) {
        return SY_RT_ERR;
    }

    (*pipec)++;
    *argc = 0;
    return SY_RT_OK;
}

sy_rt_e interp_none(char *path, sy_token_node_t *node,
                    unsigned int *pipec, unsigned int *,
                    interp_argv_t argv, pid_t pids[SY_MAX_PIDS]) {
    sy_rt_e rt;
    unsigned int pid_index = 0;
    int pipe_in_fd = STDIN_FILENO;
    int pipe_out_fd;

    unsigned int pipe_index = 0;
    do {
        int pipefd[2];
        if (pipe_index < *pipec && pipe(pipefd) == -1) {
            break;
        }

        pipe_out_fd =
            (pipe_index < *pipec) ? pipefd[1] : STDOUT_FILENO;

        rt = exec(path, argv[pipe_index], &pid_index, pids,
                  pipe_in_fd, pipe_out_fd);

        if (pipe_in_fd != STDIN_FILENO) {
            close(pipe_in_fd);
        }

        if (pipe_out_fd != STDOUT_FILENO) {
            close(pipe_out_fd);
        }

        pipe_in_fd = pipefd[0];
    } while (pipe_index++ < *pipec && rt == SY_RT_OK);

    if (rt != SY_RT_OK) {
        for (unsigned int i = 0; i < pid_index; i++) {
            kill(pids[i], SIGKILL);
        }
    }

    int stat;
    for (unsigned int i = 0; i < pid_index; i++) {
        wait(&stat);
    }

    return (rt == SY_RT_OK) ? SY_RT_END : rt;
}

sy_rt_e interp_err(char *path, sy_token_node_t *node, unsigned int *,
                   unsigned int *, interp_argv_t argv,
                   pid_t pids[SY_MAX_PIDS]) {
    return SY_RT_ERR;
}

interp_t interpreter[] = {
    [0 ... SY_TOKEN_INVALID] = interp_err,
    [SY_TOKEN_NONE] = interp_none,
    [SY_TOKEN_COMMAND] = interp_command,
    [SY_TOKEN_ARG] = interp_arg,
    [SY_TOKEN_PIPELINE] = interp_pipeline,
};

sy_rt_e sy_interp(char *path, sy_token_node_t *node) {
    pid_t pids[SY_MAX_PIDS] = {0};
    interp_argv_t argv = {0};
    unsigned int argc = 0;
    unsigned int pipec = 0;
    sy_rt_e rt;

    while ((rt = interpreter[sy_token_node_get_token(node)](
                path, node, &pipec, &argc, argv, pids)) == SY_RT_OK) {
        node = node->next;
    }

    return rt;
}