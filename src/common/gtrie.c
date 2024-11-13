#include "gtrie.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#define GTRIE_DB_SIZE 1024 * 1024 * 10  // 10MB max database size
#define UTF8_MAX_BYTES 4
#define UNICODE_MAX 0x10FFFF

static int utf8_byte_count(uint8_t first_byte) {
    if ((first_byte & 0x80) == 0) return 1;
    if ((first_byte & 0xE0) == 0xC0) return 2;
    if ((first_byte & 0xF0) == 0xE0) return 3;
    if ((first_byte & 0xF8) == 0xF0) return 4;
    return -1;  // Invalid UTF-8 leading byte
}

static uint32_t utf8_to_codepoint(const char* utf8_char, int* bytes_read) {
    const uint8_t* bytes = (const uint8_t*)utf8_char;
    int len = utf8_byte_count(bytes[0]);
    uint32_t codepoint = 0;
    
    if (len < 0) {
        *bytes_read = 1;  // Skip invalid byte
        return UINT32_MAX;
    }
    
    switch (len) {
        case 1:
            codepoint = bytes[0];
            break;
        case 2:
            codepoint = ((bytes[0] & 0x1F) << 6) | (bytes[1] & 0x3F);
            break;
        case 3:
            codepoint = ((bytes[0] & 0x0F) << 12) | ((bytes[1] & 0x3F) << 6) | (bytes[2] & 0x3F);
            break;
        case 4:
            codepoint = ((bytes[0] & 0x07) << 18) | ((bytes[1] & 0x3F) << 12) |
                       ((bytes[2] & 0x3F) << 6) | (bytes[3] & 0x3F);
            break;
    }
    
    *bytes_read = len;
    return codepoint;
}

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
    trie->node_count = 1; // Root node
    trie->doc_count = 0;
    
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
        int bytes_read;
        uint32_t codepoint = utf8_to_codepoint(word, &bytes_read);
        
        if (codepoint == UINT32_MAX || codepoint > UNICODE_MAX) {
            return EINVAL;  // Invalid UTF-8 sequence
        }
        
        // Use the codepoint as the index, you might want to add a mapping function
        // if you want to optimize the memory usage
        int index = codepoint % ALPHABET_SIZE;  // Simple mapping, you might want to improve this
        
        if (!current->children[index]) {
            current->children[index] = create_node(&err);
            if (!current->children[index]) return err;
            trie->node_count++; // Increment node count when creating new node
        }
        
        current = current->children[index];
        word += bytes_read;  // Advance by the number of bytes read
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
    
    trie->doc_count++; // Increment doc count when adding new document
    
    return 0;
}

PostingList* gtrie_search(const GTrie* trie, const char* word, int* err) {
    if (!trie || !word) {
        if (err) *err = EINVAL;
        return NULL;
    }
    
    const TrieNode* current = trie->root;
    
    while (*word) {
        int bytes_read;
        uint32_t codepoint = utf8_to_codepoint(word, &bytes_read);
        
        if (codepoint == UINT32_MAX || codepoint > UNICODE_MAX) {
            if (err) *err = EINVAL;
            return NULL;
        }
        
        int index = codepoint % ALPHABET_SIZE;
        
        if (!current->children[index]) {
            if (err) *err = ENOENT;
            return NULL;
        }
        
        current = current->children[index];
        word += bytes_read;
    }
    
    return current->postings;
}
