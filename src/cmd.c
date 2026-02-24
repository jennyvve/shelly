#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "generic.h"

typedef enum {
    IN_CMD_NONE = 0,
    IN_CMD_CD,
    IN_CMD_EXIT,
} in_cmd_e;

// Faster searching by mapping the first character to an array of
// strings containing commands with this character as the first
// character.
char *in_cmds_strs[128][1] = {
    [(int)'c'][0] = "cd",
    [(int)'e'][0] = "exit",
};

// This maps the index of the array of strings from in_cmds_strs to an
// id. Should be kept in the same order, or else, big trouble!!
in_cmd_e in_cmds_id[128][2] = {
    [(int)'c'][0] = IN_CMD_CD,
    [(int)'e'][0] = IN_CMD_EXIT,
};

in_cmd_e get_in_cmd(char *cmd) {
    char **cmds = in_cmds_strs[(int)*cmd];
    unsigned int i = 0;

    while (*cmds != NULL && !sy_cmp_str(*(cmds++), cmd)) {
        i++;
    };

    return in_cmds_id[(int)*cmd][i];
}

typedef sy_rt_e (*incmd_t)(char *path, char *argv[SY_MAX_ARG_LENGTH]);

sy_rt_e incmd_cd(char *path, char *argv[SY_MAX_ARG_LENGTH]) {
    if (argv[1] == NULL || argv[2] != NULL) {
        return SY_RT_ERR;
    }

    if (chdir(argv[1]) == -1) {
        printf("!ERR failed to change director, errno: %i.\n", errno);
        return SY_RT_ERR;
        // could probably describe what error in more detail.
    }

    size_t size = pathconf(".", _PC_PATH_MAX);
    char *buf = NULL;
    if ((buf = malloc(size)) == NULL) {
        printf("!ERR internal error, failed to allocate memory!\n");
        return SY_RT_ERR;
    }

    char *ptr = getcwd(buf, size);
    sy_str_cpy(&ptr, path, size);

    free(buf);
    return SY_RT_OK;
}

sy_rt_e incmd_exit(char *, char *[SY_MAX_ARG_LENGTH]) {
    return SY_RT_EXIT;
}

// Map command id's, found through the in_cmds_id map, to their
// appropiate function.
incmd_t in_cmds_exec[] = {
    [IN_CMD_CD] = incmd_cd,
    [IN_CMD_EXIT] = incmd_exit,
};

sy_rt_e attach_cmd_path(char *, char cmd_path[SY_MAX_ARG_LENGTH],
                        char cmd[SY_MAX_ARG_LENGTH]) {
    char *paths = getenv("PATH");
    char *ptr = paths;
    struct stat s;

    // is an absolute path?
    unsigned int i = 0;
    while (cmd[i] != '\0' && cmd[i++] != '/');
    if (cmd[i - 1] == '/') {
        sy_str_cpy(&cmd, cmd_path, SY_MAX_ARG_LENGTH);
        return (stat(cmd_path, &s) == -1) ? SY_RT_END : SY_RT_OK;
    }

    unsigned int j = 0;
    while (*ptr != '\0') {
        cmd_path[j++] = *ptr++;
        if (*ptr == ':') {
            cmd_path[j++] = '/';
            char *c = cmd;
            sy_str_cpy(&c, &cmd_path[j], SY_MAX_ARG_LENGTH - j);
            if (stat(cmd_path, &s) != -1) {
                return SY_RT_OK;
            }
            j = 0;
            ptr++;
        }
    }

    return SY_RT_ERR;
}

sy_rt_e sy_cmd_run(char *path, char *argv[SY_MAX_ARG_LENGTH]) {
    in_cmd_e cmd_id;

    if ((cmd_id = get_in_cmd(*argv)) == IN_CMD_NONE) {
        return SY_RT_NOT_FOUND;
    }

    return in_cmds_exec[cmd_id](path, argv);
}

pid_t sy_cmd_spawn(char *path, char *argv[SY_MAX_ARG_LENGTH], int in,
                   int out) {
    char cmd[SY_MAX_ARGC + 1] = {0};
    if (attach_cmd_path(path, cmd, *argv) != SY_RT_OK) {
        printf("!ERR unable to find command: %s.\n", *argv);
        return -1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);

        if (in != STDIN_FILENO) {
            dup2(in, STDIN_FILENO);
            close(in);
        }

        if (out != STDOUT_FILENO) {
            dup2(out, STDOUT_FILENO);
            close(out);
        }

        execv(cmd, argv);
        exit(EXIT_SUCCESS);
    }

    return pid;
}