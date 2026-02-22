#include "exec.h"

#include <stdbool.h>

inline bool cmp_str(char *a, char *b) {
    while (*a++ == *b++ && *a != '\0' && *b != '\0');
    return (*--a == *--b);
}

typedef sy_rt_e (*sy_exec_interp_t)(char *path,
                                    sy_token_node_t **node);

sy_exec_interp_t sy_exec_interp_err;
sy_exec_interp_t sy_exec_interp_none;
sy_exec_interp_t sy_exec_interp_command;
sy_exec_interp_t sy_exec_interp_arg;
sy_exec_interp_t sy_exec_interp_pipeline;

sy_exec_interp_t sy_interp[] = {
    [0 ... SY_TOKEN_INVALID] = sy_exec_interp_err,
    [SY_TOKEN_NONE] = sy_exec_interp_none,
    [SY_TOKEN_COMMAND] = sy_exec_interp_command,
    [SY_TOKEN_ARG] = sy_exec_interp_arg,
    [SY_TOKEN_PIPELINE] = sy_exec_interp_pipeline,
};

sy_rt_e sy_exec_interp_none(char *, sy_token_node_t **) {
    return SY_RT_OK;
}

sy_rt_e sy_exec_interp_command(char *, sy_token_node_t **) {
    return SY_RT_OK;
}

sy_rt_e sy_exec_interp_arg(char *, sy_token_node_t **) {
    return SY_RT_OK;
}

sy_rt_e sy_exec_interp_pipeline(char *, sy_token_node_t **) {
    return SY_RT_OK;
}

sy_rt_e sy_exec_interp_err(char *, sy_token_node_t **) {
    return SY_RT_ERR;
}

sy_rt_e sy_exec(char *path, sy_token_node_t *node) {
    return SY_RT_OK;
}