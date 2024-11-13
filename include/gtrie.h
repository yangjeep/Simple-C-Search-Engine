#ifndef SEARCH_ENGINE_GTRIE_H
#define SEARCH_ENGINE_GTRIE_H

#include <stddef.h>
#include <stdbool.h>
#include <errno.h>

#define TRIE_CHILDREN_SIZE 256  // Keep 256 since we'll index by bytes
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
    char* word;  // UTF-8 encoded string
} TrieNode;

typedef struct {
    TrieNode* root;
    size_t total_words;
    size_t node_count;    // Total number of nodes in the trie
    size_t doc_count;     // Total number of unique documents indexed
} GTrie;

// GTrie operations
GTrie* gtrie_create(int* err);
int gtrie_destroy(GTrie* trie);
int gtrie_insert(GTrie* trie, const char* word, const char* doc_id);
PostingList* gtrie_search(const GTrie* trie, const char* word, int* err);
char** gtrie_prefix_search(const GTrie* trie, const char* prefix, size_t* count, int* err);


#endif 