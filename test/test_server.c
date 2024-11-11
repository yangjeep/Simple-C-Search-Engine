#include "unity.h"
#include "server.h"
#include <stdio.h>

void setUp(void) {
    // Create test index file
    FILE* fp = fopen("test_index.txt", "w");
    fprintf(fp, "hello\tdoc1\t2\n");
    fprintf(fp, "world\tdoc1\t1\n");
    fprintf(fp, "hello\tdoc2\t1\n");
    fclose(fp);
}

void tearDown(void) {
    server_cleanup();
    remove("test_index.txt");
}

void test_server_init_should_load_valid_index(void) {
    // Act
    int result = server_init("test_index.txt");
    
    // Assert
    TEST_ASSERT_EQUAL_INT(0, result);
}

void test_server_init_should_handle_nonexistent_file(void) {
    // Act
    int result = server_init("nonexistent.txt");
    
    // Assert
    TEST_ASSERT_NOT_EQUAL(0, result);
}

void test_search_query_should_find_matching_documents(void) {
    // Arrange
    server_init("test_index.txt");
    
    // Act
    int num_results;
    SearchResult* results = search_query("hello", &num_results);
    
    // Assert
    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_EQUAL_INT(2, num_results);
    TEST_ASSERT_EQUAL_STRING("hello", results[0].term);
    TEST_ASSERT_EQUAL_STRING("doc1", results[0].doc_id);
    TEST_ASSERT_EQUAL_INT(2, results[0].frequency);
    TEST_ASSERT_EQUAL_STRING("hello", results[1].term);
    TEST_ASSERT_EQUAL_STRING("doc2", results[1].doc_id);
    TEST_ASSERT_EQUAL_INT(1, results[1].frequency);
}