#ifndef SEARCH_ENGINE_INDEXER_H
#define SEARCH_ENGINE_INDEXER_H

#include <stddef.h>
#include <stdbool.h>
#include <time.h>

// Forward declarations
typedef struct Indexer Indexer;
typedef struct SearchResult SearchResult;

// Search result structure
typedef struct SearchResult {
    char* doc_id;
    struct SearchResult* next;
} SearchResult;

// Create/destroy indexer
Indexer* indexer_create(void);
void indexer_destroy(Indexer* idx);

// Index operations
int indexer_add_document(Indexer* idx, const char* key, const char* doc_id);
int indexer_save(Indexer* idx, const char* filepath);
int indexer_load(Indexer* idx, const char* filepath);

// Search operations
SearchResult* indexer_search(Indexer* idx, const char* key);
void search_results_free(SearchResult* results);

// Statistics
size_t indexer_get_doc_count(const Indexer* idx);
size_t indexer_get_key_count(const Indexer* idx);
time_t indexer_get_timestamp(const Indexer* idx);

#endif // SEARCH_ENGINE_INDEXER_H 