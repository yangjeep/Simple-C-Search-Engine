#include <stdlib.h>
#include <string.h>
#include "document.h"  // Will find this in include/common due to include path

Document* document_create(const char* doc_id, const char* content) {
    Document* doc = malloc(sizeof(Document));
    if (doc == NULL) return NULL;
    
    doc->doc_id = strdup(doc_id);
    if (doc->doc_id == NULL) {
        free(doc);
        return NULL;
    }
    
    doc->content = strdup(content);
    if (doc->content == NULL) {
        free(doc->doc_id);
        free(doc);
        return NULL;
    }
    
    return doc;
}

void document_destroy(Document* doc) {
    if (doc == NULL) return;
    free(doc->doc_id);
    free(doc->content);
    free(doc);
} 