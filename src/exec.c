#include "exec.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef enum {
    IN_CMD_NONE = 0,
    IN_CMD_CD,
    IN_CMD_EXIT,
} in_cmd_e;

char *in_cmds_strs[][2] = {
    [(int)'c'][0] = "cd",
    [(int)'e'][0] = "exit",
};

in_cmd_e in_cmds_id[][2] = {
    [(int)'c'][0] = IN_CMD_CD,
    [(int)'e'][0] = IN_CMD_EXIT,
};

in_cmd_e get_in_cmd(char *cmd) {
    char **cmds = in_cmds_strs[*cmd];
    unsigned int i = 0;

    while (**(cmds + i) != '\0' && sy_cmp_str(*(cmds + i), cmd)) {
        i++;
    };

    return in_cmds_id[(int)*cmd][i];
}

typedef sy_rt_e (*incmd_t)(
    char *path, char argv[SY_MAX_ARGC + 1][SY_MAX_ARG_LENGTH]);

sy_rt_e incmd_cd(char *path,
                 char argv[SY_MAX_ARGC + 1][SY_MAX_ARG_LENGTH]) {
    if (argv[1][0] == 0 || argv[2][0] != 0) {
        return SY_RT_ERR;
    }

    if (chdir(argv[1]) == -1) {
        printf("Failed to change director, errno: %i.", errno);
        return SY_RT_ERR;
        // could probably describe what error in more detail.
    }

    size_t path_size = pathconf(".", _PC_PATH_MAX);
    sy_str_cpy(argv[1], path, path_size);
    return SY_RT_OK;
}

sy_rt_e incmd_exit(char *path,
                   char argv[SY_MAX_ARGC + 1][SY_MAX_ARG_LENGTH]) {
    return SY_RT_EXIT;
}

incmd_t in_cmds_exec[] = {
    [IN_CMD_CD] = incmd_cd,
    [IN_CMD_EXIT] = incmd_exit,
};

sy_rt_e attach_cmd_path(char *path, char cmd_path[SY_MAX_ARG_LENGTH],
                        char cmd[SY_MAX_ARG_LENGTH]) {
    char *paths = getenv("PATH");
    char *ptr = paths;
    unsigned int i = 0;

    while (cmd[i] != '\0' && cmd[i++] != '/');
    if (cmd[i - 1] == '/') {
        unsigned int s =
            sy_str_cpy(path, cmd_path, SY_MAX_ARG_LENGTH);
        sy_str_cpy(cmd, &cmd_path[s + 1], SY_MAX_ARG_LENGTH - s);
        return SY_RT_OK;
    }

    struct stat s;
    unsigned int j = 0;
    while (*ptr != '\0') {
        cmd_path[j++] = *ptr++;
        if (*ptr == ';') {
            sy_str_cpy(cmd, &cmd_path[j], SY_MAX_ARG_LENGTH - j);
            if (stat(cmd_path, &s) != -1) {
                return SY_RT_OK;
            }
            j = 0;
            ptr++;
        }
    }

    return SY_RT_ERR;
}

pid_t spawn_cmd(char *path,
                char argv[SY_MAX_ARGC + 1][SY_MAX_ARG_LENGTH], int in,
                int out) {
    char cmd[SY_MAX_ARGC + 1] = {0};
    if (attach_cmd_path(path, cmd, argv[0]) != SY_RT_OK) {
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execv(cmd, argv);
        exit(EXIT_SUCCESS);
    }

    return pid;
}

sy_rt_e exec(char *path,
             char argv[SY_MAX_ARGC + 1][SY_MAX_ARG_LENGTH],
             unsigned int *pid_index, pid_t pids[SY_MAX_PIDS]) {
    in_cmd_e cmd_id;

    if ((cmd_id = get_in_cmd(argv[0])) != IN_CMD_NONE) {
        return (in_cmds_exec[cmd_id](path, argv) != SY_RT_OK)
                   ? SY_RT_EXIT
                   : SY_RT_OK;
    }

    pids[*pid_index] = spawn_cmd(path, argv, 0, 0);
    if (pids[*pid_index] == -1) {
        return SY_RT_ERR;
    }

    (*pid_index)++;
    return SY_RT_OK;
}

typedef char *interp_argv_t[SY_MAX_PIPEC][SY_MAX_ARGC + 1];

typedef sy_rt_e (*interp_t)(char *path, sy_token_node_t *node,
                            unsigned int *pipec, unsigned int *argc,
                            interp_argv_t argv,
                            pid_t pids[SY_MAX_PIDS]);

interp_t interp_err;
interp_t interp_none;
interp_t interp_command;
interp_t interp_arg;
interp_t interp_pipeline;

interp_t sy_interp[] = {
    [0 ... SY_TOKEN_INVALID] = interp_err,
    [SY_TOKEN_NONE] = interp_none,
    [SY_TOKEN_COMMAND] = interp_command,
    [SY_TOKEN_ARG] = interp_arg,
    [SY_TOKEN_PIPELINE] = interp_pipeline,
};

sy_rt_e interp_command(char *, sy_token_node_t *node,
                       unsigned int *pipec, unsigned int *argc,
                       interp_argv_t argv, pid_t[SY_MAX_PIDS]) {
    if (*argc != 0) {
        return SY_RT_ERR;
    }

    argv[*pipec][*argc++] = node->value;
    return SY_RT_OK;
}

sy_rt_e interp_arg(char *, sy_token_node_t *node, unsigned int *pipec,
                   unsigned int *argc, interp_argv_t argv,
                   pid_t p[SY_MAX_PIDS]) {
    if (*argc >= SY_MAX_ARGC) {
        return SY_RT_ERR;
    }

    argv[*pipec][*argc++] = node->value;
    return SY_RT_OK;
}

sy_rt_e interp_pipeline(char *, sy_token_node_t *,
                        unsigned int *pipec, unsigned int *argc,
                        interp_argv_t, pid_t[SY_MAX_PIDS]) {
    if (*pipec >= SY_MAX_PIPEC) {
        return SY_RT_ERR;
    }

    (*pipec)++;
    *argc = 0;
    return SY_RT_OK;
}

sy_rt_e interp_none(char *path, sy_token_node_t *,
                    unsigned int *pipec, unsigned int *,
                    interp_argv_t argv, pid_t pids[SY_MAX_PIDS]) {
    sy_rt_e rt;
    unsigned int pid_index = 0;

    for (unsigned int pi = 0;
         pi < *pipec &&
         (rt = exec(path, argv[pi], &pid_index, pids)) == SY_RT_OK;
         pi++);

    if (rt == SY_RT_ERR) {
        for (unsigned int i = 0; i < pid_index; i++) {
            kill(pids[i], SIGKILL);
        }
    }

    int stat;
    for (unsigned int i = 0; i < pid_index; i++) {
        wait(&stat);
    }

    return SY_RT_EXIT;
}

sy_rt_e interp_err(char *, sy_token_node_t *, unsigned int *,
                   unsigned int *, interp_argv_t,
                   pid_t[SY_MAX_PIDS]) {
    return SY_RT_ERR;
}

sy_rt_e sy_exec(char *path, sy_token_node_t *node) {
    pid_t pids[SY_MAX_PIDS] = {0};
    interp_argv_t argv = {0};
    unsigned int argc = 0;
    unsigned int pipec = 0;
    sy_rt_e rt;
    sy_token_node_t *n = node;

    while ((rt = sy_interp[sy_token_node_get_token(n)](
                path, n, &pipec, &argc, argv, pids)) == SY_RT_OK) {
        n = node->next;
    }

    return (rt == SY_RT_ERR) ? SY_RT_ERR : SY_RT_OK;
}