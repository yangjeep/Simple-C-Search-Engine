#include "gtrie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define GTRIE_DB_SIZE 1024 * 1024 * 10  // 10MB max database size

static TrieNode* create_node(int* err) {
    TrieNode* node = calloc(1, sizeof(TrieNode));
    if (!node) {
        *err = ENOMEM;
        return NULL;
    }
    node->postings = NULL;
    *err = 0;
    return node;
}

static int destroy_node(TrieNode* node) {
    if (!node) {
        return EINVAL;
    }
    
    // Recursively destroy all child nodes
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            destroy_node(node->children[i]);
        }
    }
    
    if (node->postings) {
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
    return 0;
}

GTrie* gtrie_create(int* err) {
    GTrie* trie = calloc(1, sizeof(GTrie));
    if (!trie) {
        *err = ENOMEM;
        return NULL;
    }
    
    trie->root = create_node(err);
    if (!trie->root) {
        free(trie);
        return NULL;
    }
    
    trie->total_words = 0;
    
    *err = 0;
    return trie;
}


int gtrie_destroy(GTrie* trie) {
    if (!trie) return EINVAL;
    

    destroy_node(trie->root);
    
    free(trie);
    
    return 0;
}



int gtrie_insert(GTrie* trie, const char* word, const char* doc_id) {
    if (!trie || !word || !doc_id) return EINVAL;
    
    TrieNode* current = trie->root;
    int err = 0;
    
    while (*word) {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE) return EINVAL;
        
        if (!current->children[index]) {
            current->children[index] = create_node(&err);
            if (!current->children[index]) return err;
        }
        
        current = current->children[index];
        word++;
    }
    
    // Create or update posting list
    if (!current->postings) {
        current->postings = malloc(sizeof(PostingList));
        if (!current->postings) return err;
        current->postings->head = NULL;
        trie->total_words++;
    }
    
    // Add doc_id to posting list if not already present
    PostingEntry* entry = current->postings->head;
    while (entry) {
        if (strcmp(entry->doc_id, doc_id) == 0) return 0;
        entry = entry->next;
    }
    
    // Add new posting entry
    PostingEntry* new_entry = malloc(sizeof(PostingEntry));
    if (!new_entry) return err;
    
    new_entry->doc_id = strdup(doc_id);
    new_entry->next = current->postings->head;
    current->postings->head = new_entry;
    
    
    return 0;
}

PostingList* gtrie_search(const GTrie* trie, const char* word, int* err) {
    if (!trie || !word) {
        if (err) *err = EINVAL;
        return NULL;
    }
    

    const TrieNode* current = trie->root;
    
    while (*word) {
        int index = *word - 'a';
        if (index < 0 || index >= ALPHABET_SIZE || !current->children[index]) {
            if (err) *err = ENOENT;
            return NULL;
        }
        current = current->children[index];
        word++;
    }
    
    return current->postings;
}
