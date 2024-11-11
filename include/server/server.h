#ifndef SERVER_H
#define SERVER_H

#include "document.h"
#include "indexer.h"

#define MAX_QUERY_LEN 1024
#define DEFAULT_PORT 8080

typedef struct SearchResult {
    char* doc_id;
    int score;
} SearchResult;

// Initialize server with index file
int server_init(const char* index_file);

// Process search query and return results
SearchResult* search_query(const char* query, int* num_results);

// Clean up server resources
void server_cleanup(void);

#endif 