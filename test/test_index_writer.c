#include "../include/index_writer.h"
#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_process_line_basic(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);

    // Test valid line
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "apple:doc1.txt"));
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_key_count(idx));

    // Test empty line
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, ""));
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "\n"));

    // Test comment
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "# comment"));

    indexer_destroy(idx);
}

void test_process_line_invalid(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);

    // Test invalid lines
    TEST_ASSERT_EQUAL_INT(EINVAL, process_line(idx, "invalid_line"));
    TEST_ASSERT_EQUAL_INT(EINVAL, process_line(idx, NULL));

    indexer_destroy(idx);
}

void test_process_line_multiple(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);

    // Add multiple documents for same key
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "apple:doc1.txt"));
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "apple:doc2.txt"));
    TEST_ASSERT_EQUAL_size_t(2, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(1, indexer_get_key_count(idx));

    // Add different key
    TEST_ASSERT_EQUAL_INT(0, process_line(idx, "banana:doc3.txt"));
    TEST_ASSERT_EQUAL_size_t(3, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(2, indexer_get_key_count(idx));

    indexer_destroy(idx);
}

void test_process_file(void) {
    Indexer* idx = indexer_create();
    TEST_ASSERT_NOT_NULL(idx);

    // Create temporary file
    FILE* fp = tmpfile();
    TEST_ASSERT_NOT_NULL(fp);

    // Write test data
    fprintf(fp, "# Test data\n");
    fprintf(fp, "apple:doc1.txt\n");
    fprintf(fp, "banana:doc2.txt\n");
    fprintf(fp, "apple:doc3.txt\n");
    fprintf(fp, "invalid_line\n");
    fprintf(fp, "\n");
    rewind(fp);

    // Process file
    size_t processed = 0, failed = 0;
    TEST_ASSERT_EQUAL_INT(0, process_file(idx, fp, &processed, &failed));
    TEST_ASSERT_EQUAL_size_t(5, processed);
    TEST_ASSERT_EQUAL_size_t(1, failed);

    // Verify index state
    TEST_ASSERT_EQUAL_size_t(3, indexer_get_doc_count(idx));
    TEST_ASSERT_EQUAL_size_t(2, indexer_get_key_count(idx));

    fclose(fp);
    indexer_destroy(idx);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_process_line_basic);
    RUN_TEST(test_process_line_invalid);
    RUN_TEST(test_process_line_multiple);
    RUN_TEST(test_process_file);
    return UNITY_END();
} 