#pragma once

#include "mufs.h"

void
open_conn();

void
insert_file(file_t *file);

void
get_level(mufs_sqlite_data *mufs_data, int levels, char *path);

char *
resolve_title(char *artist, char *album, char *title);

char *
resolve_file(char *path, uint64_t levels);

char *
rename_file(tags_t *old, tags_t *new);

void
begin_transaction();

void
commit_transaction();
