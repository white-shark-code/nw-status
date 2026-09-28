#include "unity.h"
#include "utils.h"
#include "vec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

extern void test_vec(void);
extern void test_signals(void);
extern void test_ipv6(void);
extern void test_wireless(void);
extern void test_network(void);
extern void test_output(void);

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_vec);
    RUN_TEST(test_signals);
    RUN_TEST(test_ipv6);
    RUN_TEST(test_wireless);
    RUN_TEST(test_network);
    RUN_TEST(test_output);
    return UNITY_END();
}