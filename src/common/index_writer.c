#include "index_writer.h"
#include "logging.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int process_line(Indexer* idx, const char* line) {
    if (!idx || !line) {
        ERROR_LOG("Invalid arguments: idx=%p, line=%p", (void*)idx, (void*)line);
        return EINVAL;
    }
    // Remove leading spaces
    while (*line == ' ' || *line == '\t') {
        line++;
    }

    // Skip empty lines and comments
    if (line[0] == '\0' || line[0] == '#' || line[0] == '\n') {
        return 0;
    }

    char* line_copy = strdup(line);
    if (!line_copy) {
        ERROR_LOG("Failed to allocate memory for line");
        return ENOMEM;
    }

    int rc = EINVAL;
    char* key = line_copy;
    char* value = strchr(line_copy, ':');

    if (value) {
        // Split key and value
        *value = '\0';
        value++;

        // Trim whitespace
        char* end = value + strlen(value) - 1;
        while (end > value && (*end == '\n' || *end == '\r' || *end == ' ')) {
            *end = '\0';
            end--;
        }

        // Add to index
        DEBUG_LOG("Adding key='%s' value='%s'", key, value);
        rc = indexer_add_document(idx, key, value);
        if (rc != 0) {
            ERROR_LOG("Failed to add document: %s", strerror(rc));
        }
    } else {
        ERROR_LOG("Invalid line format (missing ':'): %s", line);
    }

    free(line_copy);
    return rc;
}

int process_file(Indexer* idx, FILE* fp, size_t* processed, size_t* failed) {
    if (!idx || !fp) {
        ERROR_LOG("Invalid arguments: idx=%p, fp=%p", (void*)idx, (void*)fp);
        return EINVAL;
    }

    char line[4096];
    size_t line_number = 0;
    size_t local_processed = 0;
    size_t local_failed = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_number++;
        
        int rc = process_line(idx, line);
        if (rc == 0) {
            local_processed++;
        } else { // Don't count comments as failures
            local_failed++;
        }

        if (line_number % 1000 == 0) {
            INFO_LOG("Processed %zu lines (%zu failed)", local_processed, local_failed);
        }
    }

    if (processed) *processed = local_processed;
    if (failed) *failed = local_failed;

    return 0;
} 