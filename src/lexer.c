
#include "lexer.h"

#include <stdbool.h>
#include <unistd.h>

#include "exec.h"

sy_rt_e lexer_text(sy_token_man_t *man, sy_token_node_t **node,
                   char **str) {
    sy_token_node_t *prev = (*node);
    *node = (*node)->next;

    switch (**str) {
        case '|':
            if (sy_token_node_get_token(prev) == SY_TOKEN_PIPELINE) {
                return SY_RT_ERR;
            }
            sy_token_node_set_token(*node, SY_TOKEN_PIPELINE);
            (*str)++;
            break;
        default:
            sy_str_cpy(str, (*node)->value, SY_MAX_ARG_LENGTH);

            if (sy_is_text(**str)) {
                return SY_RT_ERR;
            }

            sy_token_node_set_token(*node, SY_TOKEN_ARG);
            break;
    }

    return (((*node)->next = sy_token_new(man)) == NULL) ? SY_RT_ERR
                                                         : SY_RT_OK;
}

sy_rt_e lexer_whitespace(sy_token_man_t *, sy_token_node_t **,
                         char **str) {
    while (sy_is_whitespace(**str)) {
        (*str)++;
    }
    return SY_RT_OK;
}

sy_rt_e lexer_err(sy_token_man_t *, sy_token_node_t **, char **) {
    return SY_RT_ERR;
}

sy_rt_e lexer_nill(sy_token_man_t *, sy_token_node_t **, char **) {
    return SY_RT_OK;
}

sy_rt_e lexer_newline(sy_token_man_t *, sy_token_node_t **node,
                      char **) {
    if (sy_token_node_get_token(*node) == SY_TOKEN_PIPELINE) {
        return SY_RT_ERR;
    }
    return SY_RT_FOUND_NEWLINE;
}

typedef sy_rt_e (*sy_lexer_t)(sy_token_man_t *man,
                              sy_token_node_t **node, char **str);

sy_lexer_t lexer[] = {
    [0 ... 128] = lexer_text,
    ['\n'] = lexer_newline,
    [' '] = lexer_whitespace,
    ['\t'] = lexer_whitespace,
};

sy_rt_e sy_lexer(sy_token_man_t *man, sy_token_node_t *node,
                 char *str) {
    sy_rt_e rt;

    while (sy_is_whitespace(*str)) {
        str++;
    }

    if (*str == '|' || sy_is_null(*str) || sy_is_newline(*str)) {
        return SY_RT_ERR;
    }

    sy_str_cpy(&str, node->value, SY_MAX_ARG_LENGTH);
    sy_token_node_set_token(node, SY_TOKEN_COMMAND);

    if ((node->next = sy_token_new(man)) == NULL) {
        return SY_RT_ERR;
    }

    while ((rt = lexer[(int)*str](man, &node, &str)) == SY_RT_OK);

    return rt;
}