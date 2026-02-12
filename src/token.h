

#ifndef __TOKEN_H
#define __TOKEN_H

#include "generic.h"

typedef enum {
    SY_TOKEN_COMMAND = 0,
    SY_TOKEN_ARG,
    SY_TOKEN_PIPELINE,
} sy_token_e;

typedef void* sy_token_man_t;

typedef struct {
    sy_token_e token;
    char value[SY_MAX_ARG_LENGTH];
} sy_token_node_t;

#endif  // __TOKEN_H