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

extern struct mufs_data *data;

char *
val_at_level(char *path, int level)
{
	char *ptr = strtok(path, "/");
	for(size_t i = 0; i < level; i++)
		ptr = strtok(NULL, "/");
	return ptr;
}

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

void
get_level(mufs_sqlite_data *mufs_data, int levels, char *path)
{
    char query[BUFSIZE];
	char fmtstring[BUFSIZE];
    char *err;

	// There's gotta be a better way than this.
	char *restore = malloc(strlen(path) + 1);
	strcpy(restore, path);
	
	// This is SO fucking cursed
		
	// Basically, we build the format string for the current level so that we can use printf() in sqlite
	for(size_t i = 0, j = 0; i < data->opts->format[levels].specifiers; i++) {
		j += snprintf(fmtstring + j, BUFSIZE - j, ", %s", data->opts->format[levels].names[i]);	
	}

    int cx = snprintf (query, BUFSIZE, "SELECT DISTINCT printf('%s' %s) FROM FILES WHERE 1=1", 
																	data->opts->format[levels].format,
																	fmtstring);
	
	// Loop through all conditions.
	for(size_t i = 0; i < levels; i++) {
		for(size_t j = 0; j < data->opts->format[i].specifiers; j++) {
			cx += snprintf(query + cx, BUFSIZE - cx, " AND %s='%s'", data->opts->format[i].names[j], val_at_level(path, i));
			strcpy(path, restore);
		}
	}

    rc = sqlite3_exec(db, query, mufs_fill_callback, mufs_data, &err);
	free(restore);
    sqlite3_free(err);
}

char *
resolve_file(char *path, uint64_t levels)
{
    char query[BUFSIZE];

	// There's gotta be a better way than this.
	char *restore = malloc(strlen(path) + 1);
	strcpy(restore, path);

    int cx = snprintf (query, BUFSIZE, "SELECT DISTINCT Path FROM FILES WHERE 1=1 LIMIT 1");
	/*
	for(size_t i = 0; i < levels; i++) {
		for(size_t j = 0; j < data->opts->format[i].specifiers; j++) {
			//cx += snprintf(query + cx, BUFSIZE - cx, " AND %s='%s'", data->opts->format[i].names[j], val_at_level(path, i));
			strcpy(path, restore);
		}
	}
	*/

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

	char *ret = NULL;

    if((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        asprintf(&ret, "%s", (char *) sqlite3_column_text(stmt, 0));

    sqlite3_finalize(stmt);
	free(restore);
    return ret;
}

/**
 * TODO
 * Change all the tags from a specific file in the database. The
 * lookup is done by using the path from resolve_file().
 */
char *
rename_file(tags_t *old, tags_t *new)
{
    // char *path = resolve_title(old->artist, old->album, old->title);
	char *path = "";

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
