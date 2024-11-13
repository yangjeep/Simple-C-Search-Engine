#include "indexer.h"
#include "index_writer.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void print_usage(const char* program) {
    fprintf(stderr, "Usage: %s -i input_file -o output_file\n", program);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i input_file   Input file containing key:value pairs (one per line)\n");
    fprintf(stderr, "  -o output_file  Output file for the generated index\n");
    fprintf(stderr, "  -h             Show this help message\n");
}

int main(int argc, char* argv[]) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    int opt;

    // Initialize logging
    log_init("index_writer", LOG_LEVEL_INFO, LOG_DEST_STDERR);

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "i:o:h")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (!input_file || !output_file) {
        ERROR_LOG("Both input and output files must be specified");
        print_usage(argv[0]);
        return 1;
    }

    // Open input file
    FILE* fp = fopen(input_file, "r");
    if (!fp) {
        ERROR_LOG("Failed to open input file %s: %s", 
                 input_file, strerror(errno));
        return 1;
    }

    // Create indexer
    Indexer* idx = indexer_create();
    if (!idx) {
        ERROR_LOG("Failed to create indexer");
        fclose(fp);
        return 1;
    }

    // Process input file
    size_t processed = 0;
    size_t failed = 0;
    int rc = process_file(idx, fp, &processed, &failed);

    if (rc != 0) {
        ERROR_LOG("Failed to process input file: %s", strerror(rc));
        fclose(fp);
        indexer_destroy(idx);
        return 1;
    }

    INFO_LOG("Finished processing: %zu successful, %zu failed", processed, failed);

    // Save index
    INFO_LOG("Saving index to %s", output_file);
    rc = indexer_save(idx, output_file);
    if (rc != 0) {
        ERROR_LOG("Failed to save index: %s", strerror(rc));
    } else {
        INFO_LOG("Successfully saved index with %zu keys and %zu documents",
                indexer_get_key_count(idx), indexer_get_doc_count(idx));
    }

    // Cleanup
    fclose(fp);
    indexer_destroy(idx);
    log_cleanup();

    return rc;
} 