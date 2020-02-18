//
// Created by nun on 2/8/20.
//

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <taglib/tag_c.h>

#include "mufs.h"
#include "indexing.h"
#include "db.h"
#include "args.h"

struct mufs_data *data;

/**
 * Fill a tags_t object from a given path.
 */
tags_t *
get_tags_from_path(const char *path)
{
    char *fpath = NULL;
    tags_t *tags = malloc(sizeof(tags_t));
    asprintf(&fpath, "%s", path);

    tags->artist = strtok(fpath, "/");
    tags->album = strtok(NULL, "/");
    tags->title = strtok(NULL, "/");

    return tags;
}

/**
 * Calculate at which level the given path is. This is used for determining
 * whether a file is an Artist, Album or Title.
 */
int
level(const char *path)
{
    int i;
    for (i = 0; path[i]; path[i] == '/' ? i++ : *path++)
        ;
    return i;
}

/**
 * When calling a sqlite prepared select, you have to give a callback.
 * This function takes the data provided by the SELECT and fills the
 * buffer provided by FUSE with it. It is used for all levels of the
 * readdir() function.
 */
int
mufs_fill_callback(void* cdata, int argc, char** argv, char** azColName)
{
    mufs_sqlite_data *sqliteData = (mufs_sqlite_data *) cdata;

    for(int i = 0; i < argc; i++) {
        sqliteData->filler(sqliteData->buf, argv[i], NULL, 0, 0);
    }

    return 0;
}

/**
 * Initialize the file system, set config parameters.
 */
static void *
mufs_init(struct fuse_conn_info *conn,
          struct fuse_config *cfg)
{
    (void) conn;

    cfg->entry_timeout = 0;
    cfg->attr_timeout = 0;
    cfg->negative_timeout = 0;

    return NULL;
}

/**
 * Read attributes from a file. The value returned is based
 * on the level in the file system itself.
 *      Level 0 -> Artist
 *      Level 1 -> Album
 *      Level 2 -> Title
 * Artists and Albums are marked as Folders, Titles are marked
 * as symlinks to their respective files on the physical file
 * system.
 */
static int
mufs_getattr (const char *path, struct stat *stbuf,
              struct fuse_file_info *fi)
{
    (void) fi;
    int i = level(path);
    memset(stbuf, 0, sizeof(struct stat));

    if(i <= 2) {
        stbuf->st_mode = S_IFDIR;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFLNK | 0777;
        stbuf->st_nlink = 2;
    }

    lstat(path, stbuf);

    return 0;
}

/**
 * Read the contents of a directory. We fill our custom sqliteData
 * object with pointers to the buffer and the filler function.
 *
 * Based on the level in the file system, different SQL statements
 * are executed, and their contents are piped to mufs_fill_callback()
 *
 * At the end, . and .. are added to the buffer.
 */
static int
mufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi,
             enum fuse_readdir_flags flags)
{
    (void) offset;
    (void) fi;
    (void) flags;

    mufs_sqlite_data *sqliteData = (mufs_sqlite_data *) malloc(sizeof(mufs_sqlite_data));
    sqliteData->buf = buf;
    sqliteData->filler = filler;

    int i = level(path);
    // This is done as to not invalidate the const qualifier
    char *fpath = NULL;
    asprintf(&fpath, "%s", path);

    if(strcmp(path, "/") == 0) {
        get_artists(sqliteData);
    } else if (i == 1) {
        get_albums(sqliteData, fpath + 1);
    } else {
        char *artist = strtok(fpath, "/");
        char *album = strtok(NULL, "/");

        get_titles(sqliteData, artist, album);
    }

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    return 0;
}

/**
 * Read a symlink on the disk. This is used for actually playing
 * and resolving the music files. The path is retrieved from the
 * database and put into te buffer.
 */
static int
mufs_readlink(const char *path, char *buf, size_t size)
{
    tags_t *tags = get_tags_from_path(path);

    char *s = resolve_title(tags->artist, tags->album, tags->title);
    if(s != NULL) {
        strncpy(buf, s, size);
    }

    return 0;
}

/**
 * In mufs, renaming and moving files changes their tags.
 * The old tags are read from the current path, the new tags
 * are read from the path where the file is moved to, using
 * get_tags_from_path(), respectively. The file is renamed using
 * the rename_file() function, defined in db.h.
 */
static int
mufs_rename(const char *from, const char *to, unsigned int flags)
{
    tags_t *old = get_tags_from_path(from);
    tags_t *new = get_tags_from_path(to);

    char *path = rename_file(old, new);

    TagLib_File *tfile;

    if((tfile = taglib_file_new(path)) != NULL) {
        TagLib_Tag *tag = taglib_file_tag(tfile);

        taglib_tag_set_title(tag, new->title);
        taglib_tag_set_album(tag, new->album);
        taglib_tag_set_artist(tag, new->artist);

        taglib_file_save(tfile);
        taglib_file_free(tfile);
    }

    return 0;
}

static struct fuse_operations mufs_oper = {
        .init = mufs_init,
        .getattr = mufs_getattr,
        .readdir = mufs_readdir,
        .readlink = mufs_readlink,
        .rename = mufs_rename,
};

int
main(int argc, char* argv[])
{
    data = (struct mufs_data *) malloc(sizeof(struct mufs_data));
    data->opts = (struct mufs_opts *) malloc(sizeof(struct mufs_opts));

    parse_args(data, &argc, &argv);

    data->rootdir = realpath(argv[argc - 2], NULL);

    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    open_conn();
    index_files(data->rootdir);

    return fuse_main(argc, argv, &mufs_oper, NULL);
}