/**
 * @file cmd.h
 * @author Jenny Vermeltfoort (jennyvermeltfoort@outlook.com,s3787494)
 * @brief Execution of commands.
 *
 * @copyright Copyright (c) 2026
 */

#ifndef __CND_H
#define __CND_H

#include <unistd.h>

#include "generic.h"

sy_rt_e sy_cmd_run(char *path, char *argv[SY_MAX_ARG_LENGTH]);

pid_t sy_cmd_spawn(char *path, char *argv[SY_MAX_ARG_LENGTH], int in,
                   int out);

#endif  // __CND_H