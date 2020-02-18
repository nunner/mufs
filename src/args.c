//
// Created by nun on 2/16/20.
//

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "args.h"

#define MUFS_OPT(t, p, v) { t, offsetof(struct mufs_opts, p), v }

// https://github.com/libfuse/libfuse/wiki/Option-Parsing

char *mufs_version    = "0.1";
char *mufs_maintainer = "leo@leonunner.com";
char mufs_description[] = "A FUSE system based on music tags.";

static struct fuse_opt mufs_opts[] = {
        MUFS_OPT("--track", track, 1),

        FUSE_OPT_KEY("--help", MUFS_HELP),
        FUSE_OPT_KEY("--version", MUFS_VERSION),
        FUSE_OPT_END
};

static void
help()
{
    fprintf(stderr,
            "Usage: mufs [args] [root] [mountpoint]\n"
            "\nGeneral options:\n"
            "\t--help\t\tdisplay this help text\n"
            "\t--version\tdisplay the version of your mufs installation\n"
            "\nDisplay options:\n"
            "\t--track\t\tdisplay track numbers before the title\n");
}

static int
mufs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
    switch(key) {
        case MUFS_HELP:
            help();
            exit(1);
        case MUFS_VERSION:
            fprintf(stderr, "mufs version %s\n", mufs_version);
            exit(0);
    }

    return 1;
}

void
parse_args(struct mufs_data *data, int *argc, char **argv[])
{
    // Default values
    data->opts->track = false;

    struct fuse_args args = FUSE_ARGS_INIT(*argc, *argv);
    fuse_opt_parse(&args, data->opts, mufs_opts, mufs_opt_proc);

    if(*argc < 3) {
        printf("Please specify at least 2 parameters.\n\n");
        help();
        exit(1);
    }

    *argc = args.argc;
    *argv = args.argv;
}
