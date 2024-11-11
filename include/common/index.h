#ifndef SEARCH_ENGINE_INDEX_H
#define SEARCH_ENGINE_INDEX_H

#include <stddef.h>
#include "document.h"
#include "gtrie.h"

typedef struct {
    GTrie* trie;
    Document** documents;  // Array of document pointers
    size_t doc_count;
    size_t doc_capacity;
} Index;

Index* index_create(void);
void index_destroy(Index* index);
void index_add_document(Index* index, const Document* doc);
Document** index_search(Index* index, const char* query, size_t* result_count);

// New functions for prefix search
Document** index_prefix_search(Index* index, const char* prefix, size_t* result_count);

#endif 