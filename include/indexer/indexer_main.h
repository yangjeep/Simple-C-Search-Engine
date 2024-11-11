#ifndef INDEXER_MAIN_H
#define INDEXER_MAIN_H

#include "document.h"
#include "indexer.h"

// Process a document and add its terms to the index
int process_document(Indexer* indexer, Document* doc);

// Save index to a file
int save_index(Indexer* indexer, const char* filename);

#endif 