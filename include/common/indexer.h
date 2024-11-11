#ifndef INDEXER_H
#define INDEXER_H

#include "document.h"

typedef struct IndexEntry {
    char* term;
    char* doc_id;
    int frequency;
} IndexEntry;

typedef struct Indexer {
    IndexEntry** entries;
    int size;
    int capacity;
} Indexer;

Indexer* indexer_create(void);
void indexer_destroy(Indexer* indexer);
int indexer_add_term(Indexer* indexer, const char* term, const char* doc_id);
IndexEntry** indexer_find_term(Indexer* indexer, const char* term, int* count);

#endif 