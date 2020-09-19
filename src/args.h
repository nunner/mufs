#pragma once

#include "mufs.h"

/**
 * Command key definitions. Add new ones here.
 */
enum {
    MUFS_HELP   = 0x100,
    MUFS_VERSION= 0x101,
    MUFS_TARGET = 0x102
};


void
parse_args(struct mufs_data *data, int *argc, char **argv[]);
