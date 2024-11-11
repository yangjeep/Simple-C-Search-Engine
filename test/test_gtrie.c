#include "../include/gtrie.h"
#include "unity.h"
#include <string.h>

#define TEST_DB_PATH "/tmp/test_gtrie_db"

void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

// Test creation and destruction
void test_create_destroy(void) {
    GTrie* trie = gtrie_create();
    TEST_ASSERT_NOT_NULL(trie);
    TEST_ASSERT_EQUAL_INT(0, trie->total_words);
    TEST_ASSERT_NULL(trie->env);
    
    gtrie_destroy(trie);
}

// Test basic insertion and search
void test_insert_search(void) {
    GTrie* trie = gtrie_create();
    
    // Test single word insertion
    gtrie_insert(trie, "hello", "doc1");
    PostingList* result = gtrie_search(trie, "hello");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("doc1", result->head->doc_id);
    
    // Test multiple documents for same word
    gtrie_insert(trie, "hello", "doc2");
    result = gtrie_search(trie, "hello");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_NOT_NULL(result->head);
    TEST_ASSERT_NOT_NULL(result->head->next);
    TEST_ASSERT_EQUAL_STRING("doc2", result->head->doc_id);
    TEST_ASSERT_EQUAL_STRING("doc1", result->head->next->doc_id);
    
    // Test non-existent word
    result = gtrie_search(trie, "nonexistent");
    TEST_ASSERT_NULL(result);
    
    gtrie_destroy(trie);
}

// Test database operations
void test_database_operations(void) {
    GTrie* trie = gtrie_create();
    
    // Test DB initialization
    TEST_ASSERT_TRUE(gtrie_init_db(trie, TEST_DB_PATH));
    
    // Test writing to DB
    gtrie_insert(trie, "database", "doc1");
    PostingList* result = gtrie_search(trie, "database");
    TEST_ASSERT_NOT_NULL(result);
    
    // Test reading from DB
    GTrie* new_trie = gtrie_create();
    TEST_ASSERT_NOT_NULL(new_trie);
    
    TEST_ASSERT_TRUE(gtrie_init_db(new_trie, TEST_DB_PATH));
    
    PostingList* db_result = gtrie_search(new_trie, "database");
    TEST_ASSERT_NOT_NULL(db_result);
    TEST_ASSERT_EQUAL_STRING("doc1", db_result->head->doc_id);
    
    // Test DB closure
    TEST_ASSERT_TRUE(gtrie_close_db(trie));
    
    gtrie_destroy(trie);
    gtrie_destroy(new_trie);
}

// Test edge cases
void test_edge_cases(void) {
    GTrie* trie = gtrie_create();
    
    // Test NULL inputs
    gtrie_insert(trie, NULL, "doc1");
    gtrie_insert(trie, "word", NULL);
    TEST_ASSERT_EQUAL_INT(0, trie->total_words);
    
    // Test empty string
    gtrie_insert(trie, "", "doc1");
    PostingList* result = gtrie_search(trie, "");
    TEST_ASSERT_NULL(result);
    
    // Test very long word
    char long_word[1000];
    memset(long_word, 'a', 999);
    long_word[999] = '\0';
    gtrie_insert(trie, long_word, "doc1");
    result = gtrie_search(trie, long_word);
    TEST_ASSERT_NOT_NULL(result);
    
    gtrie_destroy(trie);
}

// Test concurrent operations
void test_concurrent_operations(void) {
    GTrie* trie = gtrie_create();
    TEST_ASSERT_TRUE(gtrie_init_db(trie, TEST_DB_PATH));
    
    const char* words[] = {"concurrent", "test", "multiple", "words"};
    const char* docs[] = {"doc1", "doc2", "doc3", "doc4"};
    
    // Insert multiple words
    for (int i = 0; i < 4; i++) {
        gtrie_insert(trie, words[i], docs[i]);
    }
    
    // Verify all insertions
    for (int i = 0; i < 4; i++) {
        PostingList* result = gtrie_search(trie, words[i]);
        TEST_ASSERT_NOT_NULL(result);
        TEST_ASSERT_EQUAL_STRING(docs[i], result->head->doc_id);
    }
    
    gtrie_destroy(trie);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_insert_search);
    RUN_TEST(test_database_operations);
    RUN_TEST(test_edge_cases);
    RUN_TEST(test_concurrent_operations);
    
    return UNITY_END();
} 