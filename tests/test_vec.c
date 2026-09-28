#include "unity.h"
#include "vec.h"
#include <stdlib.h>
#include <string.h>

VECTOR_DECLARE(int, Int, int)
VECTOR_DEFINE(int, Int, int)

void test_vec_init(void) {
    VecInt *vec = vec_init_int();
    TEST_ASSERT_NOT_NULL(vec);
    TEST_ASSERT_EQUAL(0, vec->size);
    TEST_ASSERT_EQUAL(0, vec->capacity);
    TEST_ASSERT_NULL(vec->data);
    vec_free_int(vec);
}

void test_vec_init_with_capacity(void) {
    VecInt *v = vec_init_int_with_capacity(16);
    TEST_ASSERT_NOT_NULL(v);
    TEST_ASSERT_EQUAL(0, v->size);
    TEST_ASSERT_EQUAL(16, v->capacity);
    TEST_ASSERT_NOT_NULL(v->data);
    vec_free_int(v);
}

void test_vec_push_single(void) {
    VecInt *vec = vec_init_int();
    TEST_ASSERT_EQUAL(0, vec_push_int(vec, 42));
    TEST_ASSERT_EQUAL(1, vec->size);
    TEST_ASSERT_EQUAL(42, vec->data[0]);
    vec_free_int(vec);
}

void test_vec_push_multiple(void) {
    VecInt *vec = vec_init_int();
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL(0, vec_push_int(vec, i));
    }
    TEST_ASSERT_EQUAL(10, vec->size);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL(i, vec->data[i]);
    }
    vec_free_int(vec);
}

void test_vec_push_resize(void) {
    VecInt *vec = vec_init_int();
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL(0, vec_push_int(vec, i));
    }
    TEST_ASSERT_EQUAL(100, vec->size);
    TEST_ASSERT_GREATER_OR_EQUAL(100, vec->capacity);
    for (int i = 0; i < 100; i++) {
        TEST_ASSERT_EQUAL(i, vec->data[i]);
    }
    vec_free_int(vec);
}

void test_vec_at_valid(void) {
    VecInt *vec = vec_init_int();
    vec_push_int(vec, 10);
    vec_push_int(vec, 20);
    int *p = vec_at_int(vec, 0);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(10, *p);
    p = vec_at_int(vec, 1);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(20, *p);
    vec_free_int(vec);
}

void test_vec_at_invalid(void) {
    VecInt *vec = vec_init_int();
    TEST_ASSERT_NULL(vec_at_int(vec, 0));
    vec_push_int(vec, 1);
    TEST_ASSERT_NULL(vec_at_int(vec, 1));
    TEST_ASSERT_NULL(vec_at_int(vec, 5));
    vec_free_int(vec);
}

void test_vec_pop_valid(void) {
    VecInt *vec = vec_init_int();
    vec_push_int(vec, 1);
    vec_push_int(vec, 2);
    vec_push_int(vec, 3);
    int *p = vec_pop_int(vec, 1);
    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_EQUAL(2, *p);
    TEST_ASSERT_EQUAL(2, vec->size);
    TEST_ASSERT_EQUAL(1, vec->data[0]);
    TEST_ASSERT_EQUAL(3, vec->data[1]);
    free(p);
    vec_free_int(vec);
}

void test_vec_pop_invalid(void) {
    VecInt *vec = vec_init_int();
    TEST_ASSERT_NULL(vec_pop_int(vec, 0));
    vec_push_int(vec, 1);
    TEST_ASSERT_NULL(vec_pop_int(vec, 1));
    vec_free_int(vec);
}

void test_vec_reserve(void) {
    VecInt *vec = vec_init_int();
    TEST_ASSERT_EQUAL(0, vec_reserve_int(vec, 32));
    TEST_ASSERT_EQUAL(32, vec->capacity);
    TEST_ASSERT_EQUAL(0, vec_reserve_int(vec, 16));
    TEST_ASSERT_EQUAL(32, vec->capacity);
    vec_free_int(vec);
}

void test_vec_free_null(void) {
    vec_free_int(NULL);
    TEST_PASS();
}

void test_vec_push_null(void) {
    TEST_ASSERT_EQUAL(-1, vec_push_int(NULL, 1));
}

void test_vec_at_null(void) {
    TEST_ASSERT_NULL(vec_at_int(NULL, 0));
}

void test_vec_pop_null(void) {
    TEST_ASSERT_NULL(vec_pop_int(NULL, 0));
}

void test_vec_reserve_null(void) {
    TEST_ASSERT_EQUAL(-1, vec_reserve_int(NULL, 10));
}

void run_vec_tests(void) {
    RUN_TEST(test_vec_init);
    RUN_TEST(test_vec_init_with_capacity);
    RUN_TEST(test_vec_push_single);
    RUN_TEST(test_vec_push_multiple);
    RUN_TEST(test_vec_push_resize);
    RUN_TEST(test_vec_at_valid);
    RUN_TEST(test_vec_at_invalid);
    RUN_TEST(test_vec_pop_valid);
    RUN_TEST(test_vec_pop_invalid);
    RUN_TEST(test_vec_reserve);
    RUN_TEST(test_vec_free_null);
    RUN_TEST(test_vec_push_null);
    RUN_TEST(test_vec_at_null);
    RUN_TEST(test_vec_pop_null);
    RUN_TEST(test_vec_reserve_null);
}

void test_vec(void) {
    run_vec_tests();
}