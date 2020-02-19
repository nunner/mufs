//
// Created by nun on 2/9/20.
//

#include <sqlite3.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>

#include "db.h"
#include "mufs.h"

const char *db_file = "~/.local/share/mufs/mufs.db";

static sqlite3 *db;
static int rc;

/**
 * Open a connection and store it in the private data field.
 * This connection is used by all other database methods.
 */
void
open_conn()
{
    wordexp_t exp_result;
    wordexp(db_file, &exp_result, 0);

    rc = sqlite3_open(exp_result.we_wordv[0], &db);
    if(rc != SQLITE_OK)
        printf("Couldn't open SQLITE databse: %s\n", sqlite3_errmsg(db));

    char *err;
    sqlite3_exec(db, "DELETE FROM FILES", NULL, NULL, &err);
    sqlite3_free(err);
    wordfree(&exp_result);
}

/**
 * Get all individual artists from the database.
 */
void
get_artists(mufs_sqlite_data *mufs_data)
{
    char *query = "SELECT DISTINCT Artist FROM FILES";
    char *err;

    rc = sqlite3_exec(db, query, mufs_fill_callback, mufs_data, &err);

    sqlite3_free(err);
}

/**
 * Get all individual albums from a specific artist.
 */
void
get_albums(mufs_sqlite_data *mufs_data, char *artist)
{
    char *query = NULL;
    asprintf(&query, "SELECT DISTINCT Album FROM FILES WHERE Artist = '%s'", artist);
    char *err;

    rc = sqlite3_exec(db, query, mufs_fill_callback, mufs_data, &err);

    sqlite3_free(err);
}

/**
 * Get all individual titles from a specific artist in a specific album.
 */
void
get_titles(mufs_sqlite_data *mufs_data, char *artist, char *album)
{
    char *query = NULL;
    asprintf(&query, "SELECT DISTINCT Title FROM FILES WHERE Artist = '%s' AND Album = '%s'", artist, album);
    char *err;

    rc = sqlite3_exec(db, query, mufs_fill_callback, mufs_data, &err);

    sqlite3_free(err);
}

/**
 * Get the physical file path from a file by giving the artist,
 * album and title tags.
 */
char *
resolve_title(char *artist, char *album, char *title)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT DISTINCT Path FROM FILES WHERE Artist = ?1 AND Album = ?2 AND Title = ?3", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, artist, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, album, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, title, -1, SQLITE_STATIC);

    char *ret = NULL;

    if((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        asprintf(&ret, (char *) sqlite3_column_text(stmt, 0), artist);

    sqlite3_finalize(stmt);
    return ret;
}

/**
 * Change all the tags from a specific file in the database. The
 * lookup is done by using the path from resolve_title().
 */
char *
rename_file(tags_t *old, tags_t *new)
{
    char *path = resolve_title(old->artist, old->album, old->title);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE FILES SET Artist = ?1, Album = ?2, Title = ?3 WHERE Path = ?4", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, new->artist, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, new->album, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, new->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, path, -1, SQLITE_STATIC);

    if((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        printf("Couldn't update database: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    return path;
}

/**
 * Insert a file object into the database. This is used when indexing
 * the mounted folder, so that values can be retrieved later easily.
 */
void
insert_file(file_t *file)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "insert into FILES (Path, Title, Album, Artist) values (?1, ?2, ?3, ?4)", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, file->path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, file->tags->title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, file->tags->album, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, file->tags->artist, -1, SQLITE_STATIC);

    if((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        printf("Couldn't insert into database: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

/**
 * Begin and start a transaction. This is done for tremendously
 * speeding up the indexing process.
 */

void
begin_transaction() {
    char* errmsg;
    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, &errmsg);

    if(errmsg != NULL) {
        printf("Couldn't begin transaction: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}

void
commit_transaction() {
    char* errmsg;
    sqlite3_exec(db, "COMMIT;", NULL, NULL, &errmsg);

    if(errmsg != NULL) {
        printf("Couldn't commit transaction: %s\n", errmsg);
        sqlite3_free(errmsg);
    }
}