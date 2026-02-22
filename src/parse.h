#ifndef __PARSE_H
#define __PARSE_H

#include "generic.h"
#include "token.h"

sy_rt_e sy_parse(sy_token_man_t *man, sy_token_node_t *node,
                 char *str);

#endif  // __PARSE_H
