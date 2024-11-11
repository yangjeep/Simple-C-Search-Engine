#ifndef SEARCH_ENGINE_GTRIE_H
#define SEARCH_ENGINE_GTRIE_H

#include <stddef.h>
#include <stdbool.h>

#define TRIE_CHILDREN_SIZE 128  // ASCII characters support
#define MAX_WORD_LENGTH 256

typedef struct PostingList {
    char* doc_id;
    int frequency;
    struct PostingList* next;
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
} GTrie;

// GTrie operations
GTrie* gtrie_create(void);
void gtrie_destroy(GTrie* trie);
void gtrie_insert(GTrie* trie, const char* word, const char* doc_id);
PostingList* gtrie_search(const GTrie* trie, const char* word);
char** gtrie_prefix_search(const GTrie* trie, const char* prefix, size_t* count);

// Serialization
bool gtrie_save_to_file(const GTrie* trie, const char* filename);
GTrie* gtrie_load_from_file(const char* filename);

#endif 