#include <stdlib.h>
#include <string.h>
#include "indexer.h"

#define INITIAL_CAPACITY 100

Indexer* indexer_create(void) {
    Indexer* indexer = malloc(sizeof(Indexer));
    if (indexer == NULL) return NULL;

    indexer->entries = malloc(sizeof(IndexEntry*) * INITIAL_CAPACITY);
    if (indexer->entries == NULL) {
        free(indexer);
        return NULL;
    }

    indexer->size = 0;
    indexer->capacity = INITIAL_CAPACITY;
    return indexer;
}

void indexer_destroy(Indexer* indexer) {
    if (indexer == NULL) return;
    
    for (int i = 0; i < indexer->size; i++) {
        free(indexer->entries[i]->term);
        free(indexer->entries[i]->doc_id);
        free(indexer->entries[i]);
    }
    
    free(indexer->entries);
    free(indexer);
}

int indexer_add_term(Indexer* indexer, const char* term, const char* doc_id) {
    if (indexer == NULL || term == NULL || doc_id == NULL) return -1;

    // Check if we need to resize
    if (indexer->size >= indexer->capacity) {
        int new_capacity = indexer->capacity * 2;
        IndexEntry** new_entries = realloc(indexer->entries, sizeof(IndexEntry*) * new_capacity);
        if (new_entries == NULL) return -1;
        
        indexer->entries = new_entries;
        indexer->capacity = new_capacity;
    }

    // Check if term already exists for this doc_id
    for (int i = 0; i < indexer->size; i++) {
        if (strcmp(indexer->entries[i]->term, term) == 0 && 
            strcmp(indexer->entries[i]->doc_id, doc_id) == 0) {
            indexer->entries[i]->frequency++;
            return 0;
        }
    }

    // Create new entry
    IndexEntry* entry = malloc(sizeof(IndexEntry));
    if (entry == NULL) return -1;

    entry->term = strdup(term);
    entry->doc_id = strdup(doc_id);
    entry->frequency = 1;

    indexer->entries[indexer->size++] = entry;
    return 0;
}

IndexEntry** indexer_find_term(Indexer* indexer, const char* term, int* count) {
    if (indexer == NULL || term == NULL || count == NULL) return NULL;

    *count = 0;
    IndexEntry** results = NULL;
    
    // Count matching entries
    for (int i = 0; i < indexer->size; i++) {
        if (strcmp(indexer->entries[i]->term, term) == 0) {
            (*count)++;
        }
    }

    if (*count == 0) return NULL;

    // Allocate and fill results array
    results = malloc(sizeof(IndexEntry*) * (*count));
    if (results == NULL) {
        *count = 0;
        return NULL;
    }

    int j = 0;
    for (int i = 0; i < indexer->size; i++) {
        if (strcmp(indexer->entries[i]->term, term) == 0) {
            results[j++] = indexer->entries[i];
        }
    }

    return results;
}