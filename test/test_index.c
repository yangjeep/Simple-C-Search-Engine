#include "unity.h"
#include "index.h"
#include <string.h>

static Indexer* indexer;

void setUp(void) {
    indexer = indexer_create();
}

void tearDown(void) {
    indexer_destroy(indexer);
}

void test_indexer_create_should_initialize_empty_indexer(void) {
    // Assert
    TEST_ASSERT_NOT_NULL(indexer);
    TEST_ASSERT_EQUAL_INT(0, indexer->size);
    TEST_ASSERT_TRUE(indexer->capacity > 0);
    TEST_ASSERT_NOT_NULL(indexer->entries);
}

void test_indexer_add_term_should_add_new_term(void) {
    // Act
    int result = indexer_add_term(indexer, "test", "doc1");
    
    // Assert
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, indexer->size);
    TEST_ASSERT_EQUAL_STRING("test", indexer->entries[0]->term);
    TEST_ASSERT_EQUAL_STRING("doc1", indexer->entries[0]->doc_id);
    TEST_ASSERT_EQUAL_INT(1, indexer->entries[0]->frequency);
}

void test_indexer_add_term_should_increment_frequency_for_existing_term(void) {
    // Arrange
    indexer_add_term(indexer, "test", "doc1");
    
    // Act
    int result = indexer_add_term(indexer, "test", "doc1");
    
    // Assert
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, indexer->size);
    TEST_ASSERT_EQUAL_INT(2, indexer->entries[0]->frequency);
}

void test_indexer_add_term_should_handle_multiple_documents(void) {
    // Act
    indexer_add_term(indexer, "test", "doc1");
    indexer_add_term(indexer, "test", "doc2");
    
    // Assert
    TEST_ASSERT_EQUAL_INT(2, indexer->size);
    TEST_ASSERT_EQUAL_STRING("doc1", indexer->entries[0]->doc_id);
    TEST_ASSERT_EQUAL_STRING("doc2", indexer->entries[1]->doc_id);
}

void test_indexer_find_term_should_find_existing_term(void) {
    // Arrange
    indexer_add_term(indexer, "test", "doc1");
    indexer_add_term(indexer, "test", "doc2");
    
    // Act
    int count;
    IndexEntry** results = indexer_find_term(indexer, "test", &count);
    
    // Assert
    TEST_ASSERT_NOT_NULL(results);
    TEST_ASSERT_EQUAL_INT(2, count);
    TEST_ASSERT_EQUAL_STRING("doc1", results[0]->doc_id);
    TEST_ASSERT_EQUAL_STRING("doc2", results[1]->doc_id);
    
}

void test_indexer_find_term_should_handle_nonexistent_term(void) {
    // Arrange
    indexer_add_term(indexer, "test", "doc1");
    indexer_add_term(indexer, "test", "doc2");
    
    // Act
    int count;
    IndexEntry** results = indexer_find_term(indexer, "nonexistent", &count);
    
    // Assert
    TEST_ASSERT_NULL(results);
    TEST_ASSERT_EQUAL_INT(0, count);
} 

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_indexer_add_term_should_handle_multiple_documents);
    RUN_TEST(test_indexer_find_term_should_find_existing_term);
    RUN_TEST(test_indexer_find_term_should_handle_nonexistent_term);
    return UNITY_END();
}
