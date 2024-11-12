#ifndef SEARCH_ENGINE_GTRIE_IO_H
#define SEARCH_ENGINE_GTRIE_IO_H

#include <lmdb.h>
#include "gtrie.h"

typedef struct {
    MDB_env* env;
    MDB_dbi dbi;
    GTrie* trie;
    const char* db_path;
    size_t db_size;
} GTrieDB;

// Create and destroy operations
GTrieDB* gtrie_db_create(const char* path, size_t db_size, int* err);
int gtrie_db_destroy(GTrieDB* db);

// Database operations
int gtrie_db_open(GTrieDB* db);
int gtrie_db_close(GTrieDB* db);
int gtrie_db_write(GTrieDB* db, const char* word, PostingList* postings);
PostingList* gtrie_db_read(const GTrieDB* db, const char* word, int* err);
int gtrie_db_sync(GTrieDB* db);

// Bulk operations
int gtrie_db_save_trie(GTrieDB* db);
int gtrie_db_load_trie(GTrieDB* db);

#endif // SEARCH_ENGINE_GTRIE_IO_H
