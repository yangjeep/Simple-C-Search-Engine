#ifndef SEARCH_ENGINE_GTRIE_H
#define SEARCH_ENGINE_GTRIE_H

#include <stddef.h>
#include <stdbool.h>
#include <lmdb.h>

#define TRIE_CHILDREN_SIZE 128  // ASCII characters support
#define MAX_WORD_LENGTH 256
#define ALPHABET_SIZE 26  

// Define the posting list structure
typedef struct PostingEntry {
    char* doc_id;
    struct PostingEntry* next;
} PostingEntry;

typedef struct PostingList {
    PostingEntry* head;
} PostingList;

typedef struct TrieNode {
    struct TrieNode* children[TRIE_CHILDREN_SIZE];
    PostingList* postings;
    bool is_end;
    char* word;
} TrieNode;

typedef struct {
    TrieNode* root;
    size_t total_words;
    MDB_env* env;
    MDB_dbi dbi;
} GTrie;

// GTrie operations
GTrie* gtrie_create(void);
void gtrie_destroy(GTrie* trie);
void gtrie_insert(GTrie* trie, const char* word, const char* doc_id);
PostingList* gtrie_search(const GTrie* trie, const char* word);
char** gtrie_prefix_search(const GTrie* trie, const char* prefix, size_t* count);

// LMDB-related operations
int gtrie_init_db(gtrie_t *trie, const char *path, size_t db_size); 
bool gtrie_close_db(GTrie* trie);
bool gtrie_write_to_db(GTrie* trie, const char* word, PostingList* postings);
PostingList* gtrie_read_from_db(const GTrie* trie, const char* word);
bool gtrie_sync_to_db(GTrie* trie);
bool gtrie_load_from_db(GTrie* trie);

#endif 