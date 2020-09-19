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

#define check(tag) {\
        if (strlen(tag) == 0) \
            tag = untagged; \
}


/*
 * Read the tags from a physical file. If the file is
 * invalid, they are discarded. If single tags are not set,
 * they are replaced with "Untagged".
 */
file_t *
get_tags(const char *fpath)
{
    file_t *file = calloc(1, sizeof(file_t));
    file->tags = calloc(1, sizeof(tags_t));

    TagLib_File *tfile;

    if((tfile = taglib_file_new(fpath)) != NULL) {
        TagLib_Tag *tag = taglib_file_tag(tfile);

		file->path = malloc(strlen(fpath) + 1);
		strcpy(file->path, fpath);

        file->tags->artist = taglib_tag_artist(tag);
		check(file->tags->artist);

        file->tags->album =  taglib_tag_album(tag);
		check(file->tags->album);

        file->tags->title = taglib_tag_title(tag);
		check(file->tags->title);

        taglib_file_free(tfile);
    }

    return file;
}

/**
 * Store a file in the database. This is only done
 * if the file has its path set.
 */
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

/**
 * Loop through all files in the root directory, parsing
 * the tags for each single one.
 */
void
index_files(char *path)
{
    int ret = 0;
    begin_transaction();
    if((ret = nftw(path, &store_file, FD_NUM, 0)) != 0)
        exit(ret);
    commit_transaction();
}
