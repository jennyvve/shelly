/**
 * @file lexer.h
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com,s3787494)
 * @brief Simple tokenizer.
 *
 * @copyright Copyright (c) 2026
 */

#ifndef __LEXER_H
#define __LEXER_H

#include "generic.h"
#include "token.h"

sy_rt_e sy_lexer(sy_token_man_t *man, sy_token_node_t *node,
                 char *str);

#endif  // __LEXER_H
