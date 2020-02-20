//
// Created by nun on 2/8/20.
//

#ifndef FUSE_TEST_MUFS_H
#define FUSE_TEST_MUFS_H

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <sqlite3.h>
#include <stdbool.h>

struct mufs_opts {
    bool track;
    char *format;
};

struct mufs_data {
    char *rootdir;
    struct mufs_opts *opts;
};

typedef struct {
    char *artist;
    char *title;
    char *album;
} tags_t;

typedef struct {
    char *path;
    tags_t *tags;
} file_t;

/**
 * This is a hack, but it works. When calling the SQLITE
 * SELECT, we can optionally pass our own data. It only works
 * through callbacks, so when filling the directory, we have to pass
 * this data for it know where and what to insert.
 */

typedef struct {
    void * buf;
    fuse_fill_dir_t filler;
} mufs_sqlite_data;

int
mufs_fill_callback(void* data, int argc, char** argv, char** azColName);

char *
translate_path(const char *path);

#endif //FUSE_TEST_MUFS_H
