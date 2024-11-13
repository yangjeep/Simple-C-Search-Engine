#ifndef SEARCH_ENGINE_INDEX_WRITER_H
#define SEARCH_ENGINE_INDEX_WRITER_H

#include "indexer.h"
#include <stdio.h>

// Process a single line of input (key:value format)
int process_line(Indexer* idx, const char* line);

// Process an entire file
int process_file(Indexer* idx, FILE* fp, size_t* processed, size_t* failed);

#endif // SEARCH_ENGINE_INDEX_WRITER_H 