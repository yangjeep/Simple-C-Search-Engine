#ifndef DOCUMENT_H
#define DOCUMENT_H

typedef struct Document {
    char* doc_id;
    char* content;
} Document;

Document* document_create(const char* doc_id, const char* content);
void document_destroy(Document* doc);

#endif 