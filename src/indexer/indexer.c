#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "indexer.h"

int process_document(Indexer* indexer, Document* doc) {
    if (!indexer || !doc) return -1;
    
    char* content = doc->content;
    char* word = strtok(content, " \t\n.,;:!?");
    
    while (word != NULL) {
        if (indexer_add_term(indexer, word, doc->doc_id) != 0) {
            return -1;
        }
        word = strtok(NULL, " \t\n.,;:!?");
    }
    
    return 0;
}

int save_index(Indexer* indexer, const char* filename) {
    if (!indexer || !filename) return -1;
    
    FILE* fp = fopen(filename, "w");
    if (!fp) return -1;
    
    for (int i = 0; i < indexer->size; i++) {
        fprintf(fp, "%s\t%s\t%d\n", 
            indexer->entries[i]->term,
            indexer->entries[i]->doc_id,
            indexer->entries[i]->frequency);
    }
    
    fclose(fp);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }
    
    // Initialize indexer
    Indexer* indexer = indexer_create();
    if (!indexer) return 1;
    
    // TODO: Read input file and process documents
    
    // Save and cleanup
    save_index(indexer, argv[2]);
    indexer_destroy(indexer);
    
    return 0;
} 