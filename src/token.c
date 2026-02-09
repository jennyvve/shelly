#include "token.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// The idea is that all token nodes are allocated in an arena (page)
// for improved data locality. Through the manager the arena's are
// allocated and nodes are added or deleted. The manager makes sure
// the correct arena is selected or allocated. Deletions are dirty,
// thus new token nodes might by dirty. Also arena's are not freed
// when empty.

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
    unsigned int free[SY_TOKEN_ARENA_MAX_TOKENS];
    unsigned int free_index;
} sy_token_arena_t;

typedef struct {
    sy_token_man_t handle;
    sy_token_arena_t* current;
    sy_token_arena_t* pool[SY_TOKEN_ARENA_MAX_ARENAS];
    unsigned int pool_index;
} sy_token_man_cont_t;

void sy_token_arena_delete(sy_token_arena_t* arena,
                           sy_token_node_t* token) {
    unsigned int index = token - arena->tokens;
    arena->free[--arena->free_index] = index;
}

sy_token_node_t* sy_token_arena_new(sy_token_arena_t* arena) {
    return &arena->tokens[arena->free[arena->free_index++]];
}

sy_token_arena_t* sy_token_arena_malloc(void) {
    sy_token_arena_t* arena = calloc(1, sizeof(sy_token_arena_t));
    if (arena != NULL) {
        for (int i = 0; i < SY_TOKEN_ARENA_MAX_TOKENS; i++) {
            arena->free[i] = i;
        }
    }
    return arena;
}

void sy_token_arena_free(sy_token_arena_t* arena) { free(arena); }

sy_rt_e sy_token_delete(sy_token_man_t* man, sy_token_node_t* token) {
    sy_token_man_cont_t* cont =
        CONTAINER_OF(man, sy_token_man_cont_t, handle);

    // find the right arena
    unsigned int i = -1;
    while ((ptrdiff_t)(token - cont->pool[++i]->tokens) >
               SY_TOKEN_ARENA_MAX_TOKENS &&
           i < cont->pool_index);

    if ((ptrdiff_t)(token - cont->pool[i]->tokens) >=
        SY_TOKEN_ARENA_MAX_TOKENS) {
        return SY_RT_ERR;
    }

    sy_token_arena_delete(cont->pool[i], token);
    return SY_RT_OK;
}

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

    for (int i = 0; sy_token_arena_is_full(cont->current) &&
                    i < cont->pool_index;
         i++) {
        cont->current = cont->pool[i];
    }

    if (cont->current->free_index >= SY_TOKEN_ARENA_MAX_TOKENS &&
        sy_token_man_new_arena(cont) != SY_RT_OK) {
        return NULL;
    }

    return sy_token_arena_new(cont->current);
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

    return cont->handle;
}

sy_token_man_free(sy_token_man_t* man) {
    sy_token_man_cont_t* cont =
        CONTAINER_OF(man, sy_token_man_cont_t, handle);

    free(cont);
}