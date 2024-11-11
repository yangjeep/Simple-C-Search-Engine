#include "unity.h"
#include "document.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_document_create_should_create_valid_document(void) {
    // Arrange
    const char* test_id = "doc123";
    const char* test_content = "This is a test document";
    
    // Act
    Document* doc = document_create(test_id, test_content);
    
    // Assert
    TEST_ASSERT_NOT_NULL(doc);
    TEST_ASSERT_NOT_NULL(doc->doc_id);
    TEST_ASSERT_NOT_NULL(doc->content);
    TEST_ASSERT_EQUAL_STRING(test_id, doc->doc_id);
    TEST_ASSERT_EQUAL_STRING(test_content, doc->content);
    
    // Cleanup
    document_destroy(doc);
}

void test_document_create_should_handle_empty_strings(void) {
    // Arrange
    const char* test_id = "";
    const char* test_content = "";
    
    // Act
    Document* doc = document_create(test_id, test_content);
    
    // Assert
    TEST_ASSERT_NOT_NULL(doc);
    TEST_ASSERT_EQUAL_STRING("", doc->doc_id);
    TEST_ASSERT_EQUAL_STRING("", doc->content);
    
    // Cleanup
    document_destroy(doc);
}

void test_document_create_should_handle_null_inputs(void) {
    // Act & Assert
    TEST_ASSERT_NULL(document_create(NULL, "content"));
    TEST_ASSERT_NULL(document_create("id", NULL));
    TEST_ASSERT_NULL(document_create(NULL, NULL));
}

void test_document_destroy_should_handle_null(void) {
    // Should not crash
    document_destroy(NULL);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_document_create_should_create_valid_document);
    RUN_TEST(test_document_create_should_handle_empty_strings);
    RUN_TEST(test_document_create_should_handle_null_inputs);
    RUN_TEST(test_document_destroy_should_handle_null);
    return UNITY_END();
} 