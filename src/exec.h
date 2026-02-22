#ifndef __EXEC_H
#define __EXEC_H

#include "generic.h"
#include "token.h"

sy_rt_e sy_exec(char* path, sy_token_node_t* node);

#endif  // __EXEC_H