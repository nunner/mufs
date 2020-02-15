//
// Created by nun on 2/9/20.
//
#define _XOPEN_SOURCE 500

#define FD_NUM 15

#include <sqlite3.h>
#include <ftw.h>
#include <stdbool.h>
#include <taglib/tag_c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indexing.h"
#include "mufs.h"
#include "db.h"

static char *untagged = "Untagged";

file_t *
get_tags(const char *fpath)
{
    file_t *file = (file_t *) calloc(1, sizeof(file_t));
    file->tags = calloc(1, sizeof(tags_t));

    TagLib_File *tfile;

    if((tfile = taglib_file_new(fpath)) != NULL) {
        TagLib_Tag *tag = taglib_file_tag(tfile);

        file->path = fpath;
        file->tags->artist = taglib_tag_artist(tag);
        if (strlen(file->tags->artist) == 0)
            file->tags->artist = untagged;

        file->tags->album =  taglib_tag_album(tag);
        if (strlen(file->tags->album) == 0)
            file->tags->album = untagged;

        file->tags->title = taglib_tag_title(tag);
        if (strlen(file->tags->title) == 0)
            file->tags->artist = basename(fpath);


        taglib_file_free(tfile);
    }

    return file;
}

static int
store_file(const char *fpath, const struct stat *sb,
             int tflag, struct FTW *ftwbuf)
{
    if(tflag == FTW_F || tflag == FTW_SL) {
        file_t *file = get_tags(fpath);
        if(file->path != NULL)
            insert_file(file);
    }
    return 0;
}

void
index_files(const char *path)
{
    int ret = 0;
    begin_transaction();
    if((ret = nftw(path, &store_file, FD_NUM, 0)) != 0)
        exit(ret);
    commit_transaction();
}
