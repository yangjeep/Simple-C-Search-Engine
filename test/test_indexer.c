#include "../include/indexer.h"
#include "unity.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/logging.h"


#define INDEXER_TEST_DIR "./Testing/Temporary/test_indexer"
#define INDEXER_TEST_FILE "./Testing/Temporary/test_indexer/test.trie"

void setUp(void) {
    struct stat st = {0};
    printf("Creating directory: %s\n", INDEXER_TEST_DIR);
    
    if (stat(INDEXER_TEST_DIR, &st) == -1) {
        int rc = mkdir(INDEXER_TEST_DIR, 0755);
        if (rc != 0) {
            printf("Failed to create directory: %s (error: %s)\n", 
                   INDEXER_TEST_DIR, strerror(rc));
        } else {
            printf("Successfully created directory\n");
        }
    } else {
        printf("Directory already exists\n");
    }
}

void tearDown(void) {
    // Clean up test files
    unlink(INDEXER_TEST_FILE);
    rmdir(INDEXER_TEST_DIR);
    log_cleanup();
}

void test_create_destroy(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    int rc = indexer_add_document(idx, "apple", "doc1");
    TEST_ASSERT_EQUAL_INT(0, rc);

    TEST_ASSERT_EQUAL_size_t(1, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_key_count(idx));
    TEST_ASSERT_GREATER_THAN(0, indexer_get_timestamp(idx));
    
    indexer_destroy(idx);
}

void test_add_documents(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Test single document
    int rc = indexer_add_document(idx, "apple", "doc1");
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_key_count(idx));
    
    // Test multiple documents for same key
    rc = indexer_add_document(idx, "apple", "doc2");
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_size_t(2, indexer_get_doc_count(idx));
    
    // Test different key
    rc = indexer_add_document(idx, "banana", "doc3");
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_EQUAL_size_t(3, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(2, indexer_get_key_count(idx));
    
    indexer_destroy(idx);
}

void test_search(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Add test documents
    indexer_add_document(idx, "apple", "doc1");
    indexer_add_document(idx, "apple", "doc2");
    indexer_add_document(idx, "banana", "doc3");
    
    // Search for existing key with multiple results
    SearchResult* results = indexer_search(idx, "apple");
    TEST_ASSERT_NOT_NULL(results);
    
    // Verify results
    TEST_ASSERT_NOT_NULL(results->doc_id);
    TEST_ASSERT_NOT_NULL(results->next);
    TEST_ASSERT_NOT_NULL(results->next->doc_id);
    TEST_ASSERT_NULL(results->next->next);
    
    // Results can be in any order, so check both combinations
    bool found_doc1 = (strcmp(results->doc_id, "doc1") == 0 && 
                      strcmp(results->next->doc_id, "doc2") == 0) ||
                     (strcmp(results->doc_id, "doc2") == 0 && 
                      strcmp(results->next->doc_id, "doc1") == 0);
    TEST_ASSERT_TRUE(found_doc1);
    
    search_results_free(results);
    
    // Search for key with single result
    results = indexer_search(idx, "banana");
    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_EQUAL_STRING("doc3", results->doc_id);
    TEST_ASSERT_NULL(results->next);
    
    search_results_free(results);
    
    // Search for non-existent key
    results = indexer_search(idx, "orange");
    TEST_ASSERT_NULL(results);
    
    indexer_destroy(idx);
}
void test_save_only(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);

    // Add test documents
    indexer_add_document(idx, "apple", "doc1");
    indexer_add_document(idx, "apple", "doc2");
    indexer_add_document(idx, "banana", "doc3");
    
    // Save index
    int rc = indexer_save(idx, INDEXER_TEST_FILE);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Verify file exists and has content
    struct stat st;
    TEST_ASSERT_EQUAL_INT(0, stat(INDEXER_TEST_FILE, &st));
    TEST_ASSERT_GREATER_THAN(0, st.st_size);

    indexer_destroy(idx);
}

void test_save_load(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    

    // Add test documents
    indexer_add_document(idx, "apple", "doc1");
    indexer_add_document(idx, "apple", "doc2");
    indexer_add_document(idx, "banana", "doc3");
    
    // Save index
    int rc = indexer_save(idx, INDEXER_TEST_FILE);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Create new indexer and load saved file
    Indexer* loaded = indexer_create();
    TEST_ASSERT_NOT_NULL(loaded);
    
    rc = indexer_load(loaded, INDEXER_TEST_FILE);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Verify loaded data
    TEST_ASSERT_EQUAL_size_t(indexer_get_doc_count(idx), 
                            indexer_get_doc_count(loaded));
    TEST_ASSERT_EQUAL_size_t(indexer_get_key_count(idx), 
                            indexer_get_key_count(loaded));
    
    // Verify search still works
    SearchResult* results = indexer_search(loaded, "apple");
    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_NOT_NULL(results->next);
    TEST_ASSERT_NULL(results->next->next);
    
    search_results_free(results);
    
    indexer_destroy(idx);
    indexer_destroy(loaded);
}

void test_error_cases(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Test NULL arguments
    TEST_ASSERT_NOT_EQUAL(0, indexer_add_document(NULL, "key", "doc"));
    TEST_ASSERT_NOT_EQUAL(0, indexer_add_document(idx, NULL, "doc"));
    TEST_ASSERT_NOT_EQUAL(0, indexer_add_document(idx, "key", NULL));
    
    TEST_ASSERT_NULL(indexer_search(NULL, "key"));
    TEST_ASSERT_NULL(indexer_search(idx, NULL));
    
    TEST_ASSERT_NOT_EQUAL(0, indexer_save(NULL, INDEXER_TEST_FILE));
    TEST_ASSERT_NOT_EQUAL(0, indexer_save(idx, NULL));
    
    TEST_ASSERT_NOT_EQUAL(0, indexer_load(NULL, INDEXER_TEST_FILE));
    TEST_ASSERT_NOT_EQUAL(0, indexer_load(idx, NULL));
    
    indexer_destroy(idx);
}

void test_save_basic(void) {
    printf("\nStarting save test\n");
    printf("Test file path: %s\n", INDEXER_TEST_FILE);
    
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Add test documents
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "test", "doc1"));
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "test", "doc2"));
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "other", "doc3"));
    
    printf("Added test documents\n");
    
    // Try to save
    int rc = indexer_save(idx, INDEXER_TEST_FILE);
    printf("Save result: %d (%s)\n", rc, strerror(rc));
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Verify file exists
    struct stat st;
    rc = stat(INDEXER_TEST_FILE, &st);
    printf("Stat result: %d (size: %ld)\n", rc, rc == 0 ? st.st_size : -1);
    TEST_ASSERT_EQUAL_INT(0, rc);
    TEST_ASSERT_GREATER_THAN(0, st.st_size);
    
    indexer_destroy(idx);
    printf("Test completed\n");
}

void test_save_and_load(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Add test documents
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "test", "doc1"));
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "test", "doc2"));
    TEST_ASSERT_EQUAL_INT(0, indexer_add_document(idx, "other", "doc3"));
    
    size_t original_doc_count = indexer_get_doc_count(idx);
    size_t original_key_count = indexer_get_key_count(idx);
    printf("Original counts - docs: %zu, keys: %zu\n", 
           original_doc_count, original_key_count);
    
    // Save
    TEST_ASSERT_EQUAL_INT(0, indexer_save(idx, INDEXER_TEST_FILE));
    
    // Create new indexer and load
    Indexer* loaded = indexer_create();
    TEST_ASSERT_NOT_NULL(loaded);
    
    int rc = indexer_load(loaded, INDEXER_TEST_FILE);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Verify counts match
    size_t loaded_doc_count = indexer_get_doc_count(loaded);
    size_t loaded_key_count = indexer_get_key_count(loaded);
    printf("Loaded counts - docs: %zu, keys: %zu\n", 
           loaded_doc_count, loaded_key_count);
    
    TEST_ASSERT_EQUAL_size_t(original_doc_count, loaded_doc_count);
    TEST_ASSERT_EQUAL_size_t(original_key_count, loaded_key_count);
    
    // Verify search still works
    SearchResult* results = indexer_search(loaded, "test");
    TEST_ASSERT_NOT_NULL(results);
    
    size_t count = 0;
    SearchResult* current = results;
    while (current) {
        count++;
        current = current->next;
    }
    TEST_ASSERT_EQUAL_size_t(2, count);
    
    search_results_free(results);
    indexer_destroy(idx);
    indexer_destroy(loaded);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_add_documents);
    RUN_TEST(test_search);
    RUN_TEST(test_save_only);
    RUN_TEST(test_save_load);
    RUN_TEST(test_error_cases);
    RUN_TEST(test_save_basic);
    RUN_TEST(test_save_and_load);
    
    return UNITY_END();
} 