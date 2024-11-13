#ifndef SEARCH_ENGINE_GTRIE_IO_H
#define SEARCH_ENGINE_GTRIE_IO_H

#include "gtrie.h"

// File metadata structure
typedef struct {
    char* filename;
    time_t timestamp;
    uint64_t doc_count;
    uint64_t node_count;
} IndexInfo;

// Core operations
int gtrie_save(const GTrie* trie, const char* directory);
GTrie* gtrie_load(const char* filepath, int* err);

// Index file management
IndexInfo* list_indices(const char* directory, size_t* count);
void free_index_info(IndexInfo* indices, size_t count);

#endif // SEARCH_ENGINE_GTRIE_IO_H
