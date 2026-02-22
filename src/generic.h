
#ifndef __GENERIC_H
#define __GENERIC_H

#define SY_MAX_ARG_LENGTH 512

typedef enum {
    SY_RT_OK = 0,
    SY_RT_ERR,
    SY_RT_FOUND_NEWLINE,
    SY_RT_EXIT,
} sy_rt_e;

#endif  // __GENERIC_H
