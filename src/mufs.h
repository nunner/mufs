#pragma once

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <sqlite3.h>
#include <stdbool.h>

#define BUFSIZE 1000

struct mufs_format {
	char **names;
	char *format;
	int specifiers;
};

struct mufs_opts {
    bool track;
	char *format_str;
	struct mufs_format *format;
	uint64_t levels;
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

/*
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
