#include "indexer.h"
#include "gtrie.h"
#include "gtrie_io.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct Indexer {
    GTrie* trie;
    time_t timestamp;
};

Indexer* indexer_create(void) {
    Indexer* idx = malloc(sizeof(Indexer));
    if (!idx) {
        ERROR_LOG("Failed to allocate Indexer");
        return NULL;
    }

    int err = 0;
    idx->trie = gtrie_create(&err);
    if (!idx->trie) {
        ERROR_LOG("Failed to create trie: %s", strerror(err));
        free(idx);
        return NULL;
    }

    idx->timestamp = time(NULL);
    DEBUG_LOG("Created new indexer instance");
    return idx;
}

void indexer_destroy(Indexer* idx) {
    if (!idx) return;

    DEBUG_LOG("Destroying indexer instance");
    gtrie_destroy(idx->trie);
    free(idx);
}

int indexer_add_document(Indexer* idx, const char* key, const char* doc_id) {
    if (!idx || !key || !doc_id) {
        ERROR_LOG("Invalid arguments: idx=%p, key=%p, doc_id=%p", 
                 (void*)idx, (void*)key, (void*)doc_id);
        return EINVAL;
    }

    TRACE_LOG("Adding document %s for key '%s'", doc_id, key);
    int rc = gtrie_insert(idx->trie, key, doc_id);
    if (rc != 0) {
        ERROR_LOG("Failed to insert key '%s': %s", key, strerror(rc));
    }

    return rc;
}

int indexer_save(Indexer* idx, const char* filepath) {
    if (!idx || !filepath) {
        ERROR_LOG("Invalid arguments: idx=%p, filepath=%p", 
                 (void*)idx, (void*)filepath);
        return EINVAL;
    }

    INFO_LOG("Saving index to %s", filepath);
    return gtrie_save(idx->trie, filepath, NULL, NULL);
}

int indexer_load(Indexer* idx, const char* filepath) {
    if (!idx || !filepath) {
        ERROR_LOG("Invalid arguments: idx=%p, filepath=%p", 
                 (void*)idx, (void*)filepath);
        return EINVAL;
    }

    INFO_LOG("Loading index from %s", filepath);
    
    int err = 0;
    GTrie* new_trie = gtrie_load(filepath, &err, NULL, NULL);
    if (!new_trie) {
        ERROR_LOG("Failed to load index: %s", strerror(err));
        return err;
    }

    // Replace existing trie
    gtrie_destroy(idx->trie);
    idx->trie = new_trie;
    idx->timestamp = time(NULL);
    
    INFO_LOG("Successfully loaded index with %zu keys", idx->trie->total_words);
    return 0;
}

SearchResult* indexer_search(Indexer* idx, const char* key) {
    if (!idx || !key) {
        ERROR_LOG("Invalid arguments: idx=%p, key=%p", (void*)idx, (void*)key);
        return NULL;
    }

    DEBUG_LOG("Searching for key '%s'", key);
    
    int err = 0;
    PostingList* postings = gtrie_search(idx->trie, key, &err);
    if (!postings) {
        if (err == ENOENT) {
            DEBUG_LOG("No results found for key '%s'", key);
        } else {
            ERROR_LOG("Search failed for key '%s': %s", key, strerror(err));
        }
        return NULL;
    }

    // Convert PostingList to SearchResult
    SearchResult* results = NULL;
    SearchResult* last = NULL;
    PostingEntry* entry = postings->head;

    while (entry) {
        SearchResult* result = malloc(sizeof(SearchResult));
        if (!result) {
            ERROR_LOG("Failed to allocate SearchResult");
            search_results_free(results);
            return NULL;
        }

        result->doc_id = strdup(entry->doc_id);
        result->next = NULL;

        if (!results) {
            results = result;
        } else {
            last->next = result;
        }
        last = result;
        entry = entry->next;
    }

    DEBUG_LOG("Found results for key '%s'", key);
    return results;
}

void search_results_free(SearchResult* results) {
    while (results) {
        SearchResult* next = results->next;
        free(results->doc_id);
        free(results);
        results = next;
    }
}

size_t indexer_get_doc_count(const Indexer* idx) {
    return idx ? idx->trie->doc_count : 0;
}

size_t indexer_get_key_count(const Indexer* idx) {
    return idx ? idx->trie->total_words : 0;
}

time_t indexer_get_timestamp(const Indexer* idx) {
    return idx ? idx->timestamp : 0;
} 