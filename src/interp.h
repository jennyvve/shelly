#ifndef __INTERP_H
#define __INTERP_H

#include "generic.h"
#include "token.h"

sy_rt_e sy_interp(char* path, sy_token_node_t* node);

#endif  // __INTERP_H