#include "gtrie_io.h"
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

// CRC32 lookup table
const uint32_t crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0E6D3D71, 0x096A6A46, 0x1C6C6162, 0x6B6B51F4, 0x1B6B1C4E, 0x66D76022, 0x0F6B6D51, 0x0E6B6B51,
    0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6,
    0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158,
    0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A,
    0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C,
    0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E,
    0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320,
    0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12,
    0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344,
    0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76,
    0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8,
    0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA,
    0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C,
    0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE,
    0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0,
    0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82,
    0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4,
    0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,
    0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278,
    0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A,
    0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C,
    0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E,
    0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

// File format header
typedef struct {
    uint32_t magic;          // Magic number for validation
    uint32_t version;        // Format version
    uint64_t timestamp;      // Creation timestamp
    uint64_t node_count;     // Total nodes
    uint64_t doc_count;      // Total unique documents
    uint32_t checksum;       // Data checksum
} IndexHeader;

static uint32_t calculate_crc32(const void* data, size_t length, uint32_t initial) {
    const uint8_t* buf = (const uint8_t*)data;
    uint32_t crc = ~initial;
    
    while (length--) {
        crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ *buf++];
    }
    
    return ~crc;
}

static void write_node_with_progress(FILE* fp, const TrieNode* node, size_t* processed, 
                                   size_t total, progress_cb progress, void* user_data) {
    if (!node) return;

    // Count and write children
    uint32_t child_count = 0;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) child_count++;
    }
    fwrite(&child_count, sizeof(uint32_t), 1, fp);

    // Write children
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            uint32_t index = i;
            fwrite(&index, sizeof(uint32_t), 1, fp);
            write_node_with_progress(fp, node->children[i], processed, total, progress, user_data);
        }
    }

    // Write postings
    uint32_t posting_count = 0;
    PostingEntry* posting = node->postings ? node->postings->head : NULL;
    while (posting) {
        posting_count++;
        posting = posting->next;
    }
    
    fwrite(&posting_count, sizeof(uint32_t), 1, fp);
    posting = node->postings ? node->postings->head : NULL;
    while (posting) {
        size_t len = strlen(posting->doc_id) + 1;
        fwrite(&len, sizeof(size_t), 1, fp);
        fwrite(posting->doc_id, 1, len, fp);
        posting = posting->next;
    }

    (*processed)++;
    if (progress) {
        progress(*processed, total, user_data);
    }
}

int gtrie_save(const GTrie* trie, const char* filepath, progress_cb progress, void* user_data) {
    if (!trie || !filepath) {
        return EINVAL;
    }

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        return errno;
    }

    IndexHeader header = {
        .magic = TRIE_MAGIC,
        .version = CURRENT_VERSION,
        .timestamp = time(NULL),
        .node_count = trie->node_count,
        .doc_count = trie->doc_count,
        .checksum = 0
    };

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        int save_errno = errno;
        fclose(fp);
        return save_errno;
    }

    size_t processed = 0;
    write_node_with_progress(fp, trie->root, &processed, trie->node_count, progress, user_data);

    // Calculate and update checksum
    long data_start = sizeof(header);
    fseek(fp, data_start, SEEK_SET);
    
    uint32_t checksum = 0;
    uint8_t buffer[8192];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        checksum = calculate_crc32(buffer, bytes_read, checksum);
    }

    header.checksum = checksum;
    fseek(fp, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, fp);

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
        if (err) *err = EINVAL;
        return NULL;
    }

    FILE* fp = fopen(filepath, "rb");
    if (!fp) {
        if (err) *err = errno;
        return NULL;
    }

    IndexHeader header;
    if (fread(&header, sizeof(header), 1, fp) != 1) {
        if (err) *err = EIO;
        fclose(fp);
        return NULL;
    }

    if (header.magic != TRIE_MAGIC) {
        if (err) *err = EINVAL;
        fclose(fp);
        return NULL;
    }

    if (header.version > CURRENT_VERSION) {
        if (err) *err = EINVAL;
        fclose(fp);
        return NULL;
    }

    // Verify checksum
    uint32_t stored_checksum = header.checksum;
    header.checksum = 0;
    
    long data_start = sizeof(header);
    fseek(fp, data_start, SEEK_SET);
    
    uint32_t calculated_checksum = 0;
    uint8_t buffer[8192];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        calculated_checksum = calculate_crc32(buffer, bytes_read, calculated_checksum);
    }

    if (calculated_checksum != stored_checksum) {
        if (err) *err = EIO;
        fclose(fp);
        return NULL;
    }

    // Reset to start of data
    fseek(fp, data_start, SEEK_SET);

    GTrie* trie = malloc(sizeof(GTrie));
    if (!trie) {
        if (err) *err = ENOMEM;
        fclose(fp);
        return NULL;
    }
    memset(trie, 0, sizeof(GTrie));

    size_t processed = 0;
    trie->root = read_node_with_progress(fp, err, &processed, header.node_count, 
                                       progress, user_data);

    if (*err) {
        free(trie);
        fclose(fp);
        return NULL;
    }

    trie->node_count = header.node_count;
    trie->doc_count = header.doc_count;

    fclose(fp);
    return trie;
}

IndexInfo* list_indices(const char* directory, size_t* count) {
    if (!directory || !count) {
        return NULL;
    }

    DIR* dir = opendir(directory);
    if (!dir) {
        *count = 0;
        return NULL;
    }

    IndexInfo* indices = NULL;
    size_t num_indices = 0;
    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        char* filepath;
        if (asprintf(&filepath, "%s/%s", directory, entry->d_name) < 0) {
            continue;
        }

        FILE* fp = fopen(filepath, "rb");
        if (fp) {
            IndexHeader header;
            if (fread(&header, sizeof(header), 1, fp) == 1 && 
                header.magic == TRIE_MAGIC) {
                
                indices = realloc(indices, (num_indices + 1) * sizeof(IndexInfo));
                if (indices) {
                    indices[num_indices].filename = strdup(entry->d_name);
                    indices[num_indices].timestamp = header.timestamp;
                    indices[num_indices].doc_count = header.doc_count;
                    indices[num_indices].node_count = header.node_count;
                    num_indices++;
                }
            }
            fclose(fp);
        }
        free(filepath);
    }

    closedir(dir);
    *count = num_indices;
    return indices;
}

void free_index_info(IndexInfo* indices, size_t count) {
    if (!indices) return;
    
    for (size_t i = 0; i < count; i++) {
        free(indices[i].filename);
    }
    free(indices);
}
