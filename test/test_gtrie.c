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

void test_large_scale(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(trie);

    // Test inserting many words from multiple documents
    const char* words[] = {
        "hello", "world", "test", "large", "scale",
        "data", "structure", "trie", "search", "index",
        "computer", "science", "algorithm", "performance", "memory"
    };
    const int num_words = sizeof(words) / sizeof(words[0]);
    
    const char* docs[] = {"doc1", "doc2", "doc3", "doc4", "doc5"};
    const int num_docs = sizeof(docs) / sizeof(docs[0]);
    
    // Insert each word into multiple documents
    for (int i = 0; i < num_words; i++) {
        for (int j = 0; j < num_docs; j++) {
            int rc = gtrie_insert(trie, words[i], docs[j]);
            TEST_ASSERT_EQUAL_INT(0, rc);
        }
    }
    
    // Verify total word count
    TEST_ASSERT_EQUAL_INT(num_words, trie->total_words);
    
    // Test searching for all words
    for (int i = 0; i < num_words; i++) {
        PostingList* result = gtrie_search(trie, words[i], &err);
        TEST_ASSERT_EQUAL_INT(0, err);
        TEST_ASSERT_NOT_NULL(result);
        
        // Verify all documents are present for this word
        PostingEntry* entry = result->head;
        int doc_count = 0;
        while (entry) {
            doc_count++;
            entry = entry->next;
        }
        TEST_ASSERT_EQUAL_INT(num_docs, doc_count);
    }
    
    // Test searching for non-existent word
    PostingList* result = gtrie_search(trie, "nonexistentword", &err);
    TEST_ASSERT_EQUAL_INT(ENOENT, err);
    TEST_ASSERT_NULL(result);
    
    int rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

// Count nodes recursively
static int count_nodes(TrieNode* node) {
    if (!node) return 0;
    int count = 1; // Count current node
    for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (node->children[i]) {
                count += count_nodes(node->children[i]);
                printf("node: %c, count: %d\n", i + 'a', count);
            }
        }
    return count;
}

void test_node_count(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(trie);

    // Insert some test words
    const char* words[] = {"cat", "cats", "catch", "caught", "dog", "dogs"};
    const int num_words = sizeof(words) / sizeof(words[0]);
    
    for (int i = 0; i < num_words; i++) {
        int rc = gtrie_insert(trie, words[i], "doc1");
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    // Count nodes in trie
    // Expected nodes:
    // Root -> c -> a -> t (3 nodes)
    //              -> c -> h (2 more nodes)
    //              -> u -> g -> h -> t (4 more nodes) 
    // Root -> d -> o -> g -> s (4 nodes)
    // Total: 13 nodes

    TEST_ASSERT_EQUAL_INT(15, count_nodes(trie->root));

    int rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}
void test_product_keywords(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(trie);

    // Insert fashion/apparel related keywords with product IDs
    const char* keywords[][2] = {
        {"cotton", "prod_001"},
        {"cotton", "prod_002"}, 
        {"tshirt", "prod_001"},
        {"casual", "prod_001"},
        {"casual", "prod_003"},
        {"denim", "prod_002"},
        {"jeans", "prod_002"},
        {"vintage", "prod_002"},
        {"vintage", "prod_003"},
        {"dress", "prod_003"}
    };
    const int num_entries = sizeof(keywords) / sizeof(keywords[0]);

    // Insert all keywords
    for (int i = 0; i < num_entries; i++) {
        int rc = gtrie_insert(trie, keywords[i][0], keywords[i][1]);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    // Test searching for keywords
    PostingList* cotton_results = gtrie_search(trie, "cotton", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(cotton_results);
    
    // Verify cotton appears in 2 products
    PostingEntry* entry = cotton_results->head;
    int cotton_count = 0;
    while (entry) {
        cotton_count++;
        entry = entry->next;
    }
    TEST_ASSERT_EQUAL_INT(2, cotton_count);

    // Test searching for "vintage" which appears in prod_002 and prod_003
    PostingList* vintage_results = gtrie_search(trie, "vintage", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(vintage_results);
    
    entry = vintage_results->head;
    bool found_prod002 = false;
    bool found_prod003 = false;
    while (entry) {
        if (strcmp(entry->doc_id, "prod_002") == 0) found_prod002 = true;
        if (strcmp(entry->doc_id, "prod_003") == 0) found_prod003 = true;
        entry = entry->next;
    }
    TEST_ASSERT_TRUE(found_prod002);
    TEST_ASSERT_TRUE(found_prod003);

    // Test searching for non-existent keyword
    PostingList* nonexistent = gtrie_search(trie, "polyester", &err);
    TEST_ASSERT_EQUAL_INT(ENOENT, err);
    TEST_ASSERT_NULL(nonexistent);

    int rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

void test_utf8_support(void) {
    int err = 0;
    GTrie* trie = gtrie_create(&err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(trie);

    // Test various UTF-8 characters
    const char* keywords[][2] = {
        {"café", "doc1"},           // 2-byte UTF-8
        {"sushi🍣", "doc2"},        // 4-byte UTF-8 emoji
        {"über", "doc3"},           // 2-byte UTF-8
        {"中文", "doc4"},           // 3-byte UTF-8
        {"привет", "doc5"}          // 2-byte UTF-8
    };
    const int num_entries = sizeof(keywords) / sizeof(keywords[0]);

    // Insert all UTF-8 keywords
    for (int i = 0; i < num_entries; i++) {
        int rc = gtrie_insert(trie, keywords[i][0], keywords[i][1]);
        TEST_ASSERT_EQUAL_INT(0, rc);
    }

    // Test searching for UTF-8 keywords
    PostingList* cafe_results = gtrie_search(trie, "café", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(cafe_results);
    TEST_ASSERT_EQUAL_STRING("doc1", cafe_results->head->doc_id);

    // Test searching for emoji
    PostingList* sushi_results = gtrie_search(trie, "sushi🍣", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(sushi_results);
    TEST_ASSERT_EQUAL_STRING("doc2", sushi_results->head->doc_id);

    // Test searching for Chinese characters
    PostingList* chinese_results = gtrie_search(trie, "中文", &err);
    TEST_ASSERT_EQUAL_INT(0, err);
    TEST_ASSERT_NOT_NULL(chinese_results);
    TEST_ASSERT_EQUAL_STRING("doc4", chinese_results->head->doc_id);

    // Test invalid UTF-8 sequence
    int rc = gtrie_insert(trie, "\xFF\xFF", "invalid");  // Invalid UTF-8
    TEST_ASSERT_EQUAL_INT(EINVAL, rc);

    rc = gtrie_destroy(trie);
    TEST_ASSERT_EQUAL_INT(0, rc);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_create_destroy);
    RUN_TEST(test_insert_search);
    RUN_TEST(test_edge_cases);
    RUN_TEST(test_large_scale);
    RUN_TEST(test_node_count);
    RUN_TEST(test_product_keywords);
    RUN_TEST(test_utf8_support);
    
    return UNITY_END();
} 