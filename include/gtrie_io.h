#ifndef SEARCH_ENGINE_GTRIE_IO_H
#define SEARCH_ENGINE_GTRIE_IO_H

#include "gtrie.h"
#include <stdint.h>
#include <time.h>

// CRC32 table for checksum calculation
extern const uint32_t crc32_table[256];

// Progress callback for save/load operations
typedef void (*progress_cb)(size_t current, size_t total, void* user_data);

// File metadata structure
typedef struct {
    char* filename;          // Name of the index file
    time_t timestamp;        // Creation time
    uint64_t doc_count;      // Number of unique documents
    uint64_t node_count;     // Number of nodes in trie
} IndexInfo;

// Core operations
int gtrie_save(const GTrie* trie, const char* filepath, progress_cb progress, void* user_data);
GTrie* gtrie_load(const char* filepath, int* err, progress_cb progress, void* user_data);

// Index file management
IndexInfo* list_indices(const char* directory, size_t* count);
void free_index_info(IndexInfo* indices, size_t count);

#endif // SEARCH_ENGINE_GTRIE_IO_H
