

#ifndef __TOKEN_H
#define __TOKEN_H

#include <stdbool.h>

#include "generic.h"

typedef enum {
    SY_TOKEN_NONE = 0,
    SY_TOKEN_COMMAND,
    SY_TOKEN_ARG,
    SY_TOKEN_PIPELINE,
    SY_TOKEN_INVALID,
} sy_token_e;

typedef void* sy_token_man_t;

typedef struct SY_TOKEN_NODE_T sy_token_node_t;
struct SY_TOKEN_NODE_T {
    sy_token_e token;
    char value[SY_MAX_ARG_LENGTH];
    sy_token_node_t* next;
};

inline sy_token_e sy_token_node_get_token(sy_token_node_t* node) {
    return node->token;
}

inline void sy_token_node_set_token(sy_token_node_t* node,
                                    sy_token_e token) {
    node->token = token;
}

sy_token_man_t* sy_token_man_malloc(void);
void sy_token_man_free(sy_token_man_t* man);
void sy_token_man_clear(sy_token_man_t* man);
sy_token_node_t* sy_token_new(sy_token_man_t* man);
sy_rt_e sy_token_delete(sy_token_man_t* man, sy_token_node_t* token);

#endif  // __TOKEN_H