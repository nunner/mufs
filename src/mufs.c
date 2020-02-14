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

struct mufs_data *data;

/**
 * Translate a FUSE path into a physical path
 * @param path
 * @return
 */
char *
translate_path(const char *path)
{
    char *ret = (char *) malloc(sizeof(char) * (strlen(path) + strlen(data->rootdir) + 1));

    strcpy(ret, data->rootdir);
    strcat(ret, path);

    return ret;
}

tags_t *
get_tags_from_path(const char *path)
{
    char *fpath = NULL;
    tags_t *tags = malloc(sizeof(tags_t));
    asprintf(&fpath, path);

    tags->artist = strtok(fpath, "/");
    tags->album = strtok(NULL, "/");
    tags->title = strtok(NULL, "/");

    return tags;
}

/**
 * Calculate at which level the given path is
 * @param path
 * @return
 */
int
level(const char *path)
{
    int i;
    for (i = 0; path[i]; path[i] == '/' ? i++ : *path++)
        ;
    return i;
}

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
 * Print the usage of the program, with the correct number of parameters
 */
void
mufs_usage()
{
    printf("Usage: ./mufs [args] [root dir] [mount point]\n");
    exit(1);
}

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
    char *fpath = NULL;
    asprintf(&fpath, path);

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

static int
mufs_open(const char *path, struct fuse_file_info *finfo)
{
    int res;
    char *fpath = translate_path(path);

    if((res = open(fpath, finfo->flags)) == -1)
        return -errno;

    close(res);
    free(fpath);
    return 0;
}

static int
mufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *finfo)
{
    int fd;
    char *fpath = translate_path(path);
    (void) finfo;

    if((fd = open(fpath, O_RDONLY)) == -1)
        return -errno;

    if(pread(fd, buf, size, offset) == -1)
        return -errno;

    close(fd);
    free(fpath);
    return 0;
}

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
    .open = mufs_open,
    .read = mufs_read,
    .rename = mufs_rename,
};

int
main(int argc, char* argv[])
{
    if(argc < 3)
        mufs_usage();

    data = (struct mufs_data *) malloc(sizeof(struct mufs_data));
    data->rootdir = realpath(argv[argc - 2], NULL);

    argv[argc-2] = argv[argc-1];
    argv[argc-1] = NULL;
    argc--;

    open_conn();
    index_files(data->rootdir);

    return fuse_main(argc, argv, &mufs_oper, NULL);
}