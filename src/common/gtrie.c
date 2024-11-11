#include "gtrie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define GTRIE_DB_SIZE 1024 * 1024 * 1024  // 1GB max database size

static TrieNode* create_node(void) {
    TrieNode* node = calloc(1, sizeof(TrieNode));
    if (!node) return NULL;
    node->postings = NULL;
    return node;
}

static void destroy_node(TrieNode* node) {
    if (!node) return;
    
    // Recursively destroy all child nodes
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            destroy_node(node->children[i]);
        }
    }
    
    // Free postings list if it exists
    if (node->postings) {
        // Free individual posting entries
        PostingEntry* current = node->postings->head;
        while (current) {
            PostingEntry* next = current->next;
            free(current->doc_id);
            free(current);
            current = next;
        }
        free(node->postings);
    }
    
    free(node);
}

GTrie* gtrie_create(void) {
    GTrie* trie = calloc(1, sizeof(GTrie));
    if (!trie) return NULL;
    
    trie->root = create_node();
    if (!trie->root) {
        free(trie);
        return NULL;
    }
    
    trie->total_words = 0;
    trie->env = NULL;
    trie->dbi = 0;
    
    return trie;
}

void gtrie_destroy(GTrie* trie) {
    if (!trie) return;
    
    // Close LMDB if it's open
    gtrie_close_db(trie);
    
    // Destroy trie structure
    destroy_node(trie->root);
    free(trie);
}

bool gtrie_init_db(GTrie* trie, const char* db_path) {
    if (!trie || !db_path) return false;
    
    int rc;
    
    // Create LMDB environment
    rc = mdb_env_create(&trie->env);
    if (rc != 0) return false;
    
    // Set database size
    rc = mdb_env_set_mapsize(trie->env, GTRIE_DB_SIZE);
    if (rc != 0) {
        mdb_env_close(trie->env);
        return false;
    }
    
    // Open environment
    rc = mdb_env_open(trie->env, db_path, 0, 0664);
    if (rc != 0) {
        mdb_env_close(trie->env);
        return false;
    }
    
    // Begin transaction
    MDB_txn* txn;
    rc = mdb_txn_begin(trie->env, NULL, 0, &txn);
    if (rc != 0) {
        mdb_env_close(trie->env);
        return false;
    }
    
    // Open database
    rc = mdb_dbi_open(txn, NULL, 0, &trie->dbi);
    if (rc != 0) {
        mdb_txn_abort(txn);
        mdb_env_close(trie->env);
        return false;
    }
    
    // Commit transaction
    rc = mdb_txn_commit(txn);
    if (rc != 0) {
        mdb_env_close(trie->env);
        return false;
    }
    
    return true;
}

bool gtrie_close_db(GTrie* trie) {
    if (!trie || !trie->env) return false;
    
    mdb_dbi_close(trie->env, trie->dbi);
    mdb_env_close(trie->env);
    trie->env = NULL;
    trie->dbi = 0;
    
    return true;
}

bool gtrie_write_to_db(GTrie* trie, const char* word, PostingList* postings) {
    if (!trie || !trie->env || !word || !postings) return false;
    
    MDB_txn* txn;
    int rc;
    
    rc = mdb_txn_begin(trie->env, NULL, 0, &txn);
    if (rc != 0) return false;
    
    // Serialize postings list to buffer
    // This is a simplified serialization - you might want to implement
    // a more sophisticated one based on your needs
    size_t buffer_size = 1024;  // Adjust based on your needs
    char* buffer = malloc(buffer_size);
    if (!buffer) {
        mdb_txn_abort(txn);
        return false;
    }
    
    // Simple serialization of posting list
    int offset = 0;
    PostingEntry* entry = postings->head;
    while (entry && offset < buffer_size - 100) {  // Leave some buffer space
        offset += snprintf(buffer + offset, buffer_size - offset,
                         "%s,", entry->doc_id);
        entry = entry->next;
    }
    
    MDB_val key = {strlen(word), (void*)word};
    MDB_val data = {offset, buffer};
    
    rc = mdb_put(txn, trie->dbi, &key, &data, 0);
    free(buffer);
    
    if (rc != 0) {
        mdb_txn_abort(txn);
        return false;
    }
    
    rc = mdb_txn_commit(txn);
    return rc == 0;
}

PostingList* gtrie_read_from_db(const GTrie* trie, const char* word) {
    if (!trie || !trie->env || !word) return NULL;
    
    MDB_txn* txn;
    int rc;
    
    rc = mdb_txn_begin(trie->env, NULL, MDB_RDONLY, &txn);
    if (rc != 0) return NULL;
    
    MDB_val key = {strlen(word), (void*)word};
    MDB_val data;
    
    rc = mdb_get(txn, trie->dbi, &key, &data);
    if (rc != 0) {
        mdb_txn_abort(txn);
        return NULL;
    }
    
    // Create new posting list
    PostingList* list = malloc(sizeof(PostingList));
    if (!list) {
        mdb_txn_abort(txn);
        return NULL;
    }
    list->head = NULL;
    
    // Parse serialized data
    char* buffer = malloc(data.mv_size + 1);
    if (!buffer) {
        free(list);
        mdb_txn_abort(txn);
        return NULL;
    }
    
    memcpy(buffer, data.mv_data, data.mv_size);
    buffer[data.mv_size] = '\0';
    
    // Parse comma-separated doc_ids
    char* token = strtok(buffer, ",");
    while (token) {
        PostingEntry* entry = malloc(sizeof(PostingEntry));
        if (!entry) break;
        
        entry->doc_id = strdup(token);
        entry->next = list->head;
        list->head = entry;
        
        token = strtok(NULL, ",");
    }
    
    free(buffer);
    mdb_txn_abort(txn);
    return list;
}

void gtrie_insert(GTrie* trie, const char* word, const char* doc_id) {
    if (!trie || !word || !doc_id) return;
    
    TrieNode* current = trie->root;
    
    // Navigate/create path for word
    while (*word) {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) return;
        
        if (!current->children[index]) {
            current->children[index] = create_node();
            if (!current->children[index]) return;
        }
        
        current = current->children[index];
        word++;
    }
    
    // Create or update posting list
    if (!current->postings) {
        current->postings = malloc(sizeof(PostingList));
        if (!current->postings) return;
        current->postings->head = NULL;
        trie->total_words++;
    }
    
    // Add doc_id to posting list if not already present
    PostingEntry* entry = current->postings->head;
    while (entry) {
        if (strcmp(entry->doc_id, doc_id) == 0) return;
        entry = entry->next;
    }
    
    // Add new posting entry
    PostingEntry* new_entry = malloc(sizeof(PostingEntry));
    if (!new_entry) return;
    
    new_entry->doc_id = strdup(doc_id);
    new_entry->next = current->postings->head;
    current->postings->head = new_entry;
    
    // Write to database if initialized
    if (trie->env) {
        gtrie_write_to_db(trie, word, current->postings);
    }
}

PostingList* gtrie_search(const GTrie* trie, const char* word) {
    if (!trie || !word) return NULL;
    
    // Try to read from database first if initialized
    if (trie->env) {
        PostingList* db_result = gtrie_read_from_db(trie, word);
        if (db_result) return db_result;
    }
    
    // Fall back to in-memory search
    const TrieNode* current = trie->root;
    
    while (*word) {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !current->children[index]) {
            return NULL;
        }
        current = current->children[index];
        word++;
    }
    
    return current->postings;
}

bool gtrie_sync_to_db(GTrie* trie) {
    // Implementation would require traversing the entire trie
    // and writing each word/posting list pair to the database
    // This is a placeholder - implement based on your needs
    return false;
}

bool gtrie_load_from_db(GTrie* trie) {
    // Implementation would require reading all entries from the database
    // and reconstructing the trie structure
    // This is a placeholder - implement based on your needs
    return false;
}
