#pragma once

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
