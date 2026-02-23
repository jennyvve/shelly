
#ifndef __GENERIC_H
#define __GENERIC_H

#include <stdbool.h>

#define SY_MAX_PIDS 128
#define SY_MAX_PIPEC 512
#define SY_MAX_ARGC 512
#define SY_MAX_ARG_LENGTH 512

typedef enum {
    SY_RT_OK = 0,
    SY_RT_ERR,
    SY_RT_FOUND_NEWLINE,
    SY_RT_EXIT,
} sy_rt_e;

inline bool sy_is_whitespace(char c) { return (c == ' '); }
inline bool sy_is_newline(char c) { return (c == '\n'); }
inline bool sy_is_null(char c) { return (c == '\0'); }
inline bool sy_is_text(char c) {
    return (!sy_is_whitespace(c) && !sy_is_newline(c) &&
            !sy_is_null(c));
}

inline unsigned int sy_str_cpy(char **src, char *trg, int size) {
    int s = 0;
    while (sy_is_text(**src) && s < size) {
        *trg++ = *(*src)++;
        s++;
    }
    *trg = '\0';
    return s;
}

inline bool sy_cmp_str(char *a, char *b) {
    while (*a++ == *b++ && *a != '\0' && *b != '\0');
    return (*--a == *--b);
}

#endif  // __GENERIC_H
