//
// Created by nun on 2/9/20.
//

#ifndef MUFS_DB_H
#define MUFS_DB_H

#include "mufs.h"

void
open_conn();

void
insert_file(file_t *file);

void
get_artists(mufs_sqlite_data *mufs_data);

void
get_albums(mufs_sqlite_data *mufs_data, char *artist);

void
get_titles(mufs_sqlite_data *mufs_data, char *artist, char *album);

char *
resolve_title(char *artist, char *album, char *title);

char *
rename_file(tags_t *old, tags_t *new);

void
begin_transaction();

void
commit_transaction();

#endif //MUFS_DB_H
