#include "../include/gtrie_io.h"
#include "../include/logging.h"
#include "unity.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Test directory and files
#define GTRIEIO_TEST_DIR "./Testing/Temporary/gtrieio_test"
#define GTRIEIO_TEST_FILE "./Testing/Temporary/gtrieio_test/test.trie"

static GTrie* create_test_trie(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    
    // Add some test data
    gtrie_insert(trie, "hello", "doc1");
    gtrie_insert(trie, "hello", "doc2");
    gtrie_insert(trie, "world", "doc1");
    gtrie_insert(trie, "test", "doc3");
    
    return trie;
}

static void progress_callback(size_t current, size_t total, void* user_data) {
    int* called = (int*)user_data;
    (*called)++;
    TEST_ASSERT_TRUE(current <= total);
}

void setUp(void) {
    // Create test directory if it doesn't exist
    struct stat st = {0};
    if (stat(GTRIEIO_TEST_DIR, &st) == -1) {
        mkdir(GTRIEIO_TEST_DIR, 0755);
    }
    log_init("test_gtrie_io", LOG_LEVEL_DEBUG, LOG_DEST_STDERR);

}

void tearDown(void) {
    // Clean up test files
    unlink(GTRIEIO_TEST_FILE);
    rmdir(GTRIEIO_TEST_DIR);
    log_cleanup();
}

void test_save_load_basic(void) {
    GTrie* original = create_test_trie();

    TEST_ASSERT_NOT_NULL(original);
    TEST_ASSERT_GREATER_THAN(0, original->node_count);
    TEST_ASSERT_GREATER_THAN(0, original->doc_count);
    TEST_ASSERT_NOT_NULL(original->root);

    
    // Save trie
    int rc = gtrie_save(original, GTRIEIO_TEST_FILE, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Verify file exists and has content
    struct stat st;
    TEST_ASSERT_EQUAL_INT(0, stat(GTRIEIO_TEST_FILE, &st));
    TEST_ASSERT_GREATER_THAN(0, st.st_size);
    
    // Load trie
    int err = 0;
    GTrie* loaded = gtrie_load(GTRIEIO_TEST_FILE, &err, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(loaded);
    
    // Verify loaded trie matches original
    TEST_ASSERT_EQUAL_INT(original->node_count, loaded->node_count);
    TEST_ASSERT_EQUAL_INT(original->doc_count, loaded->doc_count);
    
    // Verify search results
    PostingList* result = gtrie_search(loaded, "hello", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("doc1", result->head->doc_id);
    TEST_ASSERT_EQUAL_STRING("doc2", result->head->next->doc_id);
    
    gtrie_destroy(original);
    gtrie_destroy(loaded);
}

void test_save_load_empty_trie(void) {
    int err = 0;
    GTrie* empty = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(empty);
    
    // Save empty trie
    int rc = gtrie_save(empty, GTRIEIO_TEST_FILE, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Load empty trie
    GTrie* loaded = gtrie_load(GTRIEIO_TEST_FILE, &err, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(loaded);
    TEST_ASSERT_NOT_NULL(loaded->root);  // Root node should exist
    
    // Verify properties
    TEST_ASSERT_EQUAL_INT(1, loaded->node_count);  // Just the root node
    TEST_ASSERT_EQUAL_INT(0, loaded->doc_count);   // No documents
    TEST_ASSERT_EQUAL_INT(0, loaded->total_words); // No words
    
    // Try searching - should return ENOENT
    PostingList* result = gtrie_search(loaded, "test", &err);
    TEST_ASSERT_EQUAL_INT(ENOENT, err);
    TEST_ASSERT_NULL(result);
    
    gtrie_destroy(empty);
    gtrie_destroy(loaded);
}

void test_save_load_error_cases(void) {
    GTrie* trie = create_test_trie();
    int err = 0;
    
    // Test NULL filepath
    int rc = gtrie_save(trie, NULL, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(EINVAL, rc);
    
    GTrie* loaded = gtrie_load(NULL, &err, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(EINVAL, err);
    TEST_ASSERT_NULL(loaded);
    
    // Test invalid file path
    rc = gtrie_save(trie, "/nonexistent/dir/file.trie", NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(0, rc);
    
    loaded = gtrie_load("/nonexistent/file.trie", &err, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_NULL(loaded);
    
    gtrie_destroy(trie);
}

void test_list_indices(void) {
    // Create multiple test tries
    GTrie* trie1 = create_test_trie();
    GTrie* trie2 = create_test_trie();
    
    gtrie_save(trie1, GTRIEIO_TEST_DIR "/test1.trie", NULL, NULL);
    gtrie_save(trie2, GTRIEIO_TEST_DIR "/test2.trie", NULL, NULL);
    
    // List indices
    size_t count = 0;
    IndexInfo* indices = list_indices(GTRIEIO_TEST_DIR, &count);
    TEST_ASSERT_NOT_NULL(indices);
    TEST_ASSERT_EQUAL_INT(2, count);
    
    // Verify index information
    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_NOT_NULL(indices[i].filename);
        TEST_ASSERT_GREATER_THAN(0, indices[i].timestamp);
        TEST_ASSERT_EQUAL_INT(trie1->node_count, indices[i].node_count);
        TEST_ASSERT_EQUAL_INT(trie1->doc_count, indices[i].doc_count);
    }
    
    free_index_info(indices, count);
    gtrie_destroy(trie1);
    gtrie_destroy(trie2);
    
    // Test invalid directory
    indices = list_indices("/nonexistent/dir", &count);
    TEST_ASSERT_NULL(indices);
    TEST_ASSERT_EQUAL_INT(0, count);
}

void test_file_integrity(void) {
    GTrie* original = create_test_trie();
    
    // Save trie
    int rc = gtrie_save(original, GTRIEIO_TEST_FILE, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Corrupt the file
    FILE* fp = fopen(GTRIEIO_TEST_FILE, "r+b");
    TEST_ASSERT_NOT_NULL(fp);
    fseek(fp, sizeof(IndexHeader), SEEK_SET);
    char corrupt = 0xFF;
    fwrite(&corrupt, 1, 1, fp);
    fclose(fp);
    
    // Try to load corrupted file
    int err = 0;
    GTrie* loaded = gtrie_load(GTRIEIO_TEST_FILE, &err, NULL, NULL);
    TEST_ASSERT_NOT_EQUAL(0, err);
    TEST_ASSERT_NULL(loaded);
    
    gtrie_destroy(original);
}

void test_version_compatibility(void) {
    GTrie* trie = create_test_trie();
    
    // Save trie
    int rc = gtrie_save(trie, GTRIEIO_TEST_FILE, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    // Modify version number in file
    FILE* fp = fopen(GTRIEIO_TEST_FILE, "r+b");
    TEST_ASSERT_NOT_NULL(fp);
    uint32_t future_version = 99;
    fseek(fp, offsetof(IndexHeader, version), SEEK_SET);
    fwrite(&future_version, sizeof(future_version), 1, fp);
    fclose(fp);
    
    // Try to load file with future version
    int err = 0;
    GTrie* loaded = gtrie_load(GTRIEIO_TEST_FILE, &err, NULL, NULL);
    TEST_ASSERT_EQUAL_INT(EINVAL, err);
    TEST_ASSERT_NULL(loaded);
    
    gtrie_destroy(trie);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_save_load_basic);
    RUN_TEST(test_save_load_empty_trie);
    RUN_TEST(test_save_load_error_cases);
    RUN_TEST(test_list_indices);
    RUN_TEST(test_file_integrity);
    RUN_TEST(test_version_compatibility);
    
    return UNITY_END();
} 