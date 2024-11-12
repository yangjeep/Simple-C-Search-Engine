#include "../include/gtrie.h"
#include "unity.h"
#include <string.h>
#include <stdlib.h>

void setUp(void) {
}

void tearDown(void) {
    // Called after each test
}

void test_create_destroy(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(trie);
    TEST_ASSERT_EQUAL_INT(0, trie->total_words);
    
    int rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

void test_insert_search(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    
    // Test single word insertion
    int rc = gtrie_insert(trie, "hello", "doc1");
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    PostingList* result = gtrie_search(trie, "hello", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("doc1", result->head->doc_id);
    
    // Test multiple documents for same word
    rc = gtrie_insert(trie, "hello", "doc2");
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    result = gtrie_search(trie, "hello", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(result->head);
    TEST_ASSERT_NOT_NULL(result->head->next);
    TEST_ASSERT_EQUAL_STRING("doc2", result->head->doc_id);
    TEST_ASSERT_EQUAL_STRING("doc1", result->head->next->doc_id);
    
    // Test non-existent word
    result = gtrie_search(trie, "nonexistent", &err);
    TEST_ASSERT_EQUAL_INT(ENOENT, err);
    TEST_ASSERT_NULL(result);
    
    rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

void test_edge_cases(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    
    // Test NULL inputs
    int rc = gtrie_insert(trie, NULL, "doc1");
    TEST_ASSERT_EQUAL_INT(EINVAL, rc);
    
    rc = gtrie_insert(trie, "word", NULL);
    TEST_ASSERT_EQUAL_INT(EINVAL, rc);
    
    TEST_ASSERT_EQUAL_INT(0, trie->total_words);
    
    // Test empty string
    rc = gtrie_insert(trie, "", "doc1");
    TEST_ASSERT_EQUAL_INT(0, rc);
    
    PostingList* result = gtrie_search(trie, "", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(result);
    
    rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_insert_search);
    RUN_TEST(test_edge_cases);
    
    return UNITY_END();
} 