//
// Created by nun on 2/16/20.
//

#ifndef MUFS_ARGS_H
#define MUFS_ARGS_H

#include "mufs.h"

/**
 * Command key definitions. Add new ones here.
 */
static enum {
    MUFS_HELP   = 0x100,
    MUFS_VERSION= 0x101,
    MUFS_TARGET = 0x102
};


void
parse_args(struct mufs_data *data, int *argc, char **argv[]);

#endif //MUFS_ARGS_H
