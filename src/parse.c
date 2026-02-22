
#include "parse.h"

#include <stdbool.h>
#include <unistd.h>

#include "exec.h"

inline bool is_whitespace(char c) { return (c == ' '); }
inline bool is_newline(char c) { return (c == '\n'); }
inline bool is_null(char c) { return (c == '\0'); }
inline bool is_text(char c) {
    return (!is_whitespace(c) && !is_newline(c) && !is_null(c));
}
inline void str_cpy(char **src, char *trg, int size) {
    while (is_text(**src) && --size) {
        *trg++ = *(*src)++;
    }
}

sy_rt_e sy_parse_text(sy_token_man_t *man, sy_token_node_t *node,
                      char **str) {
    switch (**str) {
        case '|':
            sy_token_node_set_token(node, SY_TOKEN_PIPELINE);
            break;
        default:
            str_cpy(str, node->value, SY_MAX_ARG_LENGTH);

            if (is_text(**str)) {
                return SY_RT_ERR;
            }

            sy_token_node_set_token(node, SY_TOKEN_ARG);
            break;
    }

    return ((node->next = sy_token_new(man)) == NULL) ? SY_RT_ERR
                                                      : SY_RT_OK;
}
sy_rt_e sy_parse_whitespace(sy_token_man_t *, sy_token_node_t *,
                            char **str) {
    while (is_whitespace(*(++(*str))));
    return SY_RT_OK;
}

sy_rt_e sy_parse_err(sy_token_man_t *, sy_token_node_t *, char **) {
    return SY_RT_ERR;
}

sy_rt_e sy_parse_nill(sy_token_man_t *, sy_token_node_t *, char **) {
    return SY_RT_OK;
}

sy_rt_e sy_parse_newline(sy_token_man_t *, sy_token_node_t *,
                         char **) {
    return SY_RT_FOUND_NEWLINE;
}

typedef sy_rt_e (*sy_parse_t)(sy_token_man_t *man,
                              sy_token_node_t *node, char **str);

sy_parse_t sy_parser[] = {
    [0 ... 128] = sy_parse_err,    ['\0'] = sy_parse_nill,
    ['\n'] = sy_parse_newline,     ['a' ... 'z'] = sy_parse_text,
    ['A' ... 'Z'] = sy_parse_text, [' '] = sy_parse_whitespace,
    ['\t'] = sy_parse_whitespace,
};

sy_rt_e sy_parse(sy_token_man_t *man, sy_token_node_t *node,
                 char *str) {
    sy_rt_e rt;

    if (*str == '|') {
        return SY_RT_ERR;
    }

    str_cpy(&str, node->value, SY_MAX_ARG_LENGTH);
    sy_token_node_set_token(node, SY_TOKEN_COMMAND);

    if ((node->next = sy_token_new(man)) == NULL) {
        return SY_RT_ERR;
    }

    do {
        node = node->next;
    } while ((rt = sy_parser[(int)*str](man, node, &str)) ==
             SY_RT_OK);

    return rt;
}