/**
 * @file token.c
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com,s3787494)
 * @brief Simple arena manager
 *
 * @copyright Copyright (c) 2026
 */

#include "token.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// The idea is that all token nodes are allocated in an arena (page)
// for improved data locality. The manager manages multiple pages and
// will make sure that new tokens are retrieved from undistributed
// places inside the pages. It will allocate pages on the go.
// Resets are dirty, thus new token have to be cleared during
// retrieval. Could improve this logic by freeing arena pages that
// haven't beeen used in a while.

#define OFFSET_OF(type, member) ((size_t) & ((type*)0)->member)
#define CONTAINER_OF(ptr, type, member)                      \
    ({                                                       \
        const __typeof__(((type*)0)->member)* _mptr = (ptr); \
        (type*)((char*)_mptr - OFFSET_OF(type, member));     \
    })

#define SY_TOKEN_ARENA_MAX_ARENAS \
    128  // shouldn't exceed type size of sy_token_man_cont_t->free
#define SY_TOKEN_ARENA_MAX_TOKENS 1024

typedef struct {
    sy_token_node_t tokens[SY_TOKEN_ARENA_MAX_TOKENS];
    unsigned int index;
} sy_token_arena_t;

typedef struct {
    sy_token_man_t handle;
    sy_token_arena_t* current;
    sy_token_arena_t* pool[SY_TOKEN_ARENA_MAX_ARENAS];
    unsigned int pool_index;
} sy_token_man_cont_t;

sy_token_node_t* sy_token_retrieve_new(sy_token_arena_t* arena) {
    return &arena->tokens[arena->index++];
}

sy_token_arena_t* sy_token_arena_malloc(void) {
    return calloc(1, sizeof(sy_token_arena_t));
}

void sy_token_arena_free(sy_token_arena_t* arena) { free(arena); }

sy_rt_e sy_token_man_new_arena(sy_token_man_cont_t* cont) {
    if (cont->pool_index == SY_TOKEN_ARENA_MAX_ARENAS) {
        return SY_RT_ERR;
    }

    sy_token_arena_t* new;
    if ((new = sy_token_arena_malloc()) == NULL) {
        return SY_RT_ERR;
    }

    cont->pool[cont->pool_index++] = new;
    cont->current = new;
    return SY_RT_OK;
}

sy_token_node_t* sy_token_new(sy_token_man_t* man) {
    sy_token_man_cont_t* cont =
        CONTAINER_OF(man, sy_token_man_cont_t, handle);

    if (cont->current->index >= SY_TOKEN_ARENA_MAX_TOKENS &&
        sy_token_man_new_arena(cont) != SY_RT_OK) {
        return NULL;
    }

    sy_token_node_t* node = sy_token_retrieve_new(cont->current);
    memset(node, 0, sizeof(sy_token_node_t));
    return node;
}

sy_token_man_t* sy_token_man_malloc(void) {
    sy_token_man_cont_t* cont;

    if ((cont = calloc(1, sizeof(sy_token_man_cont_t))) == NULL) {
        return NULL;
    }

    if (sy_token_man_new_arena(cont) != SY_RT_OK) {
        free(cont);
        return NULL;
    }

    return &cont->handle;
}

void sy_token_man_clear(sy_token_man_t* man) {
    sy_token_man_cont_t* cont =
        CONTAINER_OF(man, sy_token_man_cont_t, handle);

    for (unsigned int i = 0; i < cont->pool_index; i++) {
        cont->pool[i]->index = 0;
    }
}

void sy_token_man_free(sy_token_man_t* man) {
    sy_token_man_cont_t* cont =
        CONTAINER_OF(man, sy_token_man_cont_t, handle);

    for (unsigned int i = 0; i < cont->pool_index; i++) {
        sy_token_arena_free(cont->pool[i]);
    }
    free(cont);
}