#include "../include/indexer.h"
#include "unity.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../include/logging.h"

#define TEST_INDEX_DIR "./Testing/Temporary/test_index"
#define TEST_INDEX_FILE "./Testing/Temporary/test_index/test.trie"

void setUp(void) {
    // Create test directory
    struct stat st = {0};
    if (stat(TEST_INDEX_DIR, &st) == -1) {
        mkdir(TEST_INDEX_DIR, 0755);
    }
    log_init("test_indexer", LOG_LEVEL_DEBUG, LOG_DEST_STDERR);
}

void tearDown(void) {
    // Clean up test files
    unlink(TEST_INDEX_FILE);
    rmdir(TEST_INDEX_DIR);
    log_cleanup();
}

void test_create_destroy(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    TEST_ASSERT_EQUAL_size_t(0, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(0, indexer_get_key_count(idx));
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

void test_save_load(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);
    
    // Add test documents
    indexer_add_document(idx, "apple", "doc1");
    indexer_add_document(idx, "apple", "doc2");
    indexer_add_document(idx, "banana", "doc3");
    
    // Save index
    int rc = indexer_save(idx, TEST_INDEX_FILE);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Create new indexer and load saved file
    Indexer* loaded = indexer_create();
    TEST_ASSERT_NOT_NULL(loaded);
    
    rc = indexer_load(loaded, TEST_INDEX_FILE);
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
    
    TEST_ASSERT_NOT_EQUAL(0, indexer_save(NULL, TEST_INDEX_FILE));
    TEST_ASSERT_NOT_EQUAL(0, indexer_save(idx, NULL));
    
    TEST_ASSERT_NOT_EQUAL(0, indexer_load(NULL, TEST_INDEX_FILE));
    TEST_ASSERT_NOT_EQUAL(0, indexer_load(idx, NULL));
    
    indexer_destroy(idx);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_add_documents);
    RUN_TEST(test_search);
    RUN_TEST(test_save_load);
    RUN_TEST(test_error_cases);
    
    return UNITY_END();
} 