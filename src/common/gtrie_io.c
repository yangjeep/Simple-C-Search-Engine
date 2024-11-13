#define _GNU_SOURCE
#include "gtrie_io.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define TRIE_MAGIC 0x45495254  // "TRIE" in hex
#define CURRENT_VERSION 1

static void write_node_with_progress(FILE* fp, const TrieNode* node, size_t* processed, 
                                   size_t total, progress_cb progress, void* user_data) {
    if (!node) {
        TRACE_LOG("Skipping NULL node");
        return;
    }

    // Count and write children
    uint32_t child_count = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) child_count++;
    }
    
    TRACE_LOG("Writing node with %u children", child_count);
    if (fwrite(&child_count, sizeof(uint32_t), 1, fp) != 1) {
        ERROR_LOG("Failed to write child_count: %s", strerror(errno));
        return;
    }

    // Write children
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            uint32_t index = i;
            if (fwrite(&index, sizeof(uint32_t), 1, fp) != 1) {
                ERROR_LOG("Failed to write child index %d: %s", i, strerror(errno));
                return;
            }
            write_node_with_progress(fp, node->children[i], processed, total, progress, user_data);
        }
    }

    // Count and write postings
    uint32_t posting_count = 0;
    PostingEntry* posting = node->postings ? node->postings->head : NULL;
    while (posting) {
        posting_count++;
        posting = posting->next;
    }
    
    TRACE_LOG("Writing %u postings", posting_count);
    if (fwrite(&posting_count, sizeof(uint32_t), 1, fp) != 1) {
        ERROR_LOG("Failed to write posting_count: %s", strerror(errno));
        return;
    }

    posting = node->postings ? node->postings->head : NULL;
    while (posting) {
        size_t len = strlen(posting->doc_id) + 1;
        if (fwrite(&len, sizeof(size_t), 1, fp) != 1) {
            ERROR_LOG("Failed to write doc_id length %zu: %s", len, strerror(errno));
            return;
        }
        if (fwrite(posting->doc_id, 1, len, fp) != len) {
            ERROR_LOG("Failed to write doc_id content: %s", strerror(errno));
            return;
        }
        TRACE_LOG("Wrote doc_id: %s", posting->doc_id);
        posting = posting->next;
    }

    (*processed)++;
    if (progress) {
        progress(*processed, total, user_data);
    }
}

int gtrie_save(const GTrie* trie, const char* filepath, progress_cb progress, void* user_data) {
    if (!trie || !filepath) {
        ERROR_LOG("Invalid arguments: trie=%p, filepath=%p", (void*)trie, (void*)filepath);
        return EINVAL;
    }

    INFO_LOG("Saving trie to %s (nodes: %zu, docs: %zu, words: %zu)", 
             filepath, trie->node_count, trie->doc_count, trie->total_words);

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        ERROR_LOG("Failed to open file %s for writing: %s", filepath, strerror(errno));
        return errno;
    }

    IndexHeader header = {
        .magic = TRIE_MAGIC,
        .version = CURRENT_VERSION,
        .timestamp = time(NULL),
        .node_count = trie->node_count,
        .doc_count = trie->doc_count,
        .total_words = trie->total_words
    };

    DEBUG_LOG("Writing header: magic=0x%x, version=%u, timestamp=%lu", 
              header.magic, header.version, header.timestamp);

    size_t written = fwrite(&header, sizeof(header), 1, fp);
    if (written != 1) {
        ERROR_LOG("Failed to write header: %s", strerror(errno));
        int save_errno = errno;
        fclose(fp);
        return save_errno;
    }

    size_t processed = 0;
    DEBUG_LOG("Starting to write trie nodes...");
    write_node_with_progress(fp, trie->root, &processed, trie->node_count, progress, user_data);
    DEBUG_LOG("Finished writing %zu nodes", processed);

    INFO_LOG("Successfully saved trie to %s", filepath);
    fclose(fp);
    return 0;
}

static TrieNode* read_node_with_progress(FILE* fp, int* err, size_t* processed,
                                       size_t total, progress_cb progress, void* user_data) {
    TrieNode* node = malloc(sizeof(TrieNode));
    if (!node) {
        *err = ENOMEM;
        return NULL;
    }
    memset(node, 0, sizeof(TrieNode));

    uint32_t child_count;
    if (fread(&child_count, sizeof(uint32_t), 1, fp) != 1) {
        *err = EIO;
        free(node);
        return NULL;
    }

    // Read children
    for (uint32_t i = 0; i < child_count; i++) {
        uint32_t index;
        if (fread(&index, sizeof(uint32_t), 1, fp) != 1) {
            *err = EIO;
            free(node);
            return NULL;
        }
        node->children[index] = read_node_with_progress(fp, err, processed, total, progress, user_data);
        if (*err) {
            free(node);
            return NULL;
        }
    }

    // Read postings
    uint32_t posting_count;
    if (fread(&posting_count, sizeof(uint32_t), 1, fp) != 1) {
        *err = EIO;
        free(node);
        return NULL;
    }

    if (posting_count > 0) {
        node->postings = malloc(sizeof(PostingList));
        if (!node->postings) {
            *err = ENOMEM;
            free(node);
            return NULL;
        }
        node->postings->head = NULL;
    }

    for (uint32_t i = 0; i < posting_count; i++) {
        size_t len;
        if (fread(&len, sizeof(size_t), 1, fp) != 1) {
            *err = EIO;
            free(node->postings);
            free(node);
            return NULL;
        }

        char* doc_id = malloc(len);
        if (!doc_id) {
            *err = ENOMEM;
            free(node->postings);
            free(node);
            return NULL;
        }

        if (fread(doc_id, 1, len, fp) != len) {
            free(doc_id);
            free(node->postings);
            free(node);
            *err = EIO;
            return NULL;
        }

        // Create new posting entry
        PostingEntry* entry = malloc(sizeof(PostingEntry));
        if (!entry) {
            free(doc_id);
            free(node->postings);
            free(node);
            *err = ENOMEM;
            return NULL;
        }
        entry->doc_id = doc_id;
        entry->next = node->postings->head;
        node->postings->head = entry;
    }

    (*processed)++;
    if (progress) {
        progress(*processed, total, user_data);
    }

    return node;
}

GTrie* gtrie_load(const char* filepath, int* err, progress_cb progress, void* user_data) {
    if (!filepath) {
        ERROR_LOG("Invalid filepath argument (NULL)");
        if (err) *err = EINVAL;
        return NULL;
    }

    INFO_LOG("Loading trie from %s", filepath);

    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        ERROR_LOG("Failed to open file %s: %s", filepath, strerror(errno));
        if (err) *err = errno;
        return NULL;
    }

    IndexHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        ERROR_LOG("Failed to read index header: %s", strerror(errno));
        if (err) *err = EIO;
        fclose(fp);
        return NULL;
    }

    if (header.magic != TRIE_MAGIC) {
        ERROR_LOG("Invalid magic number in file %s: expected 0x%x, got 0x%x", 
                  filepath, TRIE_MAGIC, header.magic);
        if (err) *err = EINVAL;
        fclose(fp);
        return NULL;
    }

    if (header.version > CURRENT_VERSION) {
        ERROR_LOG("Unsupported version in file %s: expected %u, got %u", 
                  filepath, CURRENT_VERSION, header.version);
        if (err) *err = EINVAL;
        fclose(fp);
        return NULL;
    }

    GTrie* trie = malloc(sizeof(GTrie));
    if (!trie) {
        ERROR_LOG("Failed to allocate GTrie structure");
        if (err) *err = ENOMEM;
        fclose(fp);
        return NULL;
    }
    memset(trie, 0, sizeof(GTrie));

    trie->node_count = header.node_count;
    trie->doc_count = header.doc_count;
    trie->total_words = header.total_words;

    size_t processed = 0;
    trie->root = read_node_with_progress(fp, err, &processed, header.node_count, 
                                       progress, user_data);

    if (*err) {
        free(trie);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    return trie;
}

IndexInfo* list_indices(const char* directory, size_t* count) {
    if (!directory || !count) {
        ERROR_LOG("Invalid arguments: directory=%p, count=%p", (void*)directory, (void*)count);
        return NULL;
    }

    INFO_LOG("Listing indices in directory: %s", directory);

    DIR* dir = opendir(directory);
    if (!dir) {
        ERROR_LOG("Failed to open directory %s: %s", directory, strerror(errno));
        *count = 0;
        return NULL;
    }

    IndexInfo* indices = NULL;
    size_t num_indices = 0;
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        char* filepath;
        if (asprintf(&filepath, "%s/%s", directory, entry->d_name) < 0) {
            ERROR_LOG("Failed to allocate filepath for %s", entry->d_name);
            continue;
        }

        FILE* fp = fopen(filepath, "rb");
        if (fp) {
            IndexHeader header;
            if (fread(&header, sizeof(header), 1, fp) == 1 && 
                header.magic == TRIE_MAGIC) {
                
                DEBUG_LOG("Found valid index: %s (nodes: %zu, docs: %zu)", 
                         entry->d_name, header.node_count, header.doc_count);
                
                indices = realloc(indices, (num_indices + 1) * sizeof(IndexInfo));
                if (indices) {
                    indices[num_indices].filename = strdup(entry->d_name);
                    indices[num_indices].timestamp = header.timestamp;
                    indices[num_indices].doc_count = header.doc_count;
                    indices[num_indices].node_count = header.node_count;
                    num_indices++;
                } else {
                    ERROR_LOG("Failed to reallocate indices array");
                }
            } else {
                TRACE_LOG("Skipping invalid file: %s", entry->d_name);
            }
            fclose(fp);
        } else {
            TRACE_LOG("Could not open file %s: %s", filepath, strerror(errno));
        }
        free(filepath);
    }

    closedir(dir);
    *count = num_indices;
    INFO_LOG("Found %zu valid indices", num_indices);
    return indices;
}

void free_index_info(IndexInfo* indices, size_t count) {
    if (!indices) {
        TRACE_LOG("NULL indices pointer, nothing to free");
        return;
    }
    
    DEBUG_LOG("Freeing %zu index info structures", count);
    for (size_t i = 0; i < count; i++) {
        free(indices[i].filename);
    }
    free(indices);
}
