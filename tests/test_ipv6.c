#include "unity.h"
#include "utils.h"
#include "config.h"
#include <string.h>

void test_should_show_ipv6_type_multicast(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_type("multicast"));
}

void test_should_show_ipv6_type_link_local(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_type("link-local"));
}

void test_should_show_ipv6_type_ula(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_type("ULA"));
}

void test_should_show_ipv6_type_global(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_type("global"));
}

void test_should_show_ipv6_type_unknown(void) {
    TEST_ASSERT_EQUAL(0, should_show_ipv6_type("unknown"));
    TEST_ASSERT_EQUAL(0, should_show_ipv6_type(NULL));
    TEST_ASSERT_EQUAL(0, should_show_ipv6_type(""));
}

void test_should_show_ipv6_scope_unlimited(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(0));
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(1));
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(100));
}

void test_should_show_ipv6_scope_limited(void) {
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(0));
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(5));
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(6));
    TEST_ASSERT_EQUAL(1, should_show_ipv6_scope(100));
}

void test_get_ipv6_icon_multicast(void) {
    IPv6 ip = { .type = "multicast" };
    TEST_ASSERT_EQUAL_STRING(IPV6_ICON_MULTICAST, get_ipv6_icon(&ip));
}

void test_get_ipv6_icon_link_local(void) {
    IPv6 ip = { .type = "link-local" };
    TEST_ASSERT_EQUAL_STRING(IPV6_ICON_LINK_LOCAL, get_ipv6_icon(&ip));
}

void test_get_ipv6_icon_ula(void) {
    IPv6 ip = { .type = "ULA" };
    TEST_ASSERT_EQUAL_STRING(IPV6_ICON_ULA, get_ipv6_icon(&ip));
}

void test_get_ipv6_icon_global(void) {
    IPv6 ip = { .type = "global" };
    TEST_ASSERT_EQUAL_STRING(IPV6_ICON_GLOBAL, get_ipv6_icon(&ip));
}

void test_get_ipv6_icon_null(void) {
    TEST_ASSERT_EQUAL_STRING(DEFAULT_IPV6_ICON, get_ipv6_icon(NULL));
}

void test_get_ipv6_icon_unknown_type(void) {
    IPv6 ip = { .type = "unknown" };
    TEST_ASSERT_EQUAL_STRING(IPV6_ICON_GLOBAL, get_ipv6_icon(&ip));
}

void test_get_ipv6_icon_color_multicast(void) {
    IPv6 ip = { .type = "multicast" };
    TEST_ASSERT_EQUAL_STRING("#ff8800", get_ipv6_icon_color(&ip));
}

void test_get_ipv6_icon_color_link_local(void) {
    IPv6 ip = { .type = "link-local" };
    TEST_ASSERT_EQUAL_STRING("#ffff00", get_ipv6_icon_color(&ip));
}

void test_get_ipv6_icon_color_ula(void) {
    IPv6 ip = { .type = "ULA" };
    TEST_ASSERT_EQUAL_STRING("#00ffff", get_ipv6_icon_color(&ip));
}

void test_get_ipv6_icon_color_global(void) {
    IPv6 ip = { .type = "global" };
    TEST_ASSERT_EQUAL_STRING("#ff8800", get_ipv6_icon_color(&ip));
}

void test_get_ipv6_icon_color_null(void) {
    TEST_ASSERT_EQUAL_STRING(DEFAULT_IPV6_ICON_COLOR, get_ipv6_icon_color(NULL));
}

void test_get_ipv4_icon(void) {
    TEST_ASSERT_EQUAL_STRING(DEFAULT_IPV4_ICON, get_ipv4_icon());
}

void test_get_ipv4_icon_color(void) {
    TEST_ASSERT_EQUAL_STRING(DEFAULT_IPV4_ICON_COLOR, get_ipv4_icon_color());
}

void test_interface_status_str(void) {
    Wireless wire_up = { .status = "0000" };
    Wireless wire_down = { .status = "down" };
    Wireless wire_other = { .status = "0000" };
    
    TEST_ASSERT_EQUAL_STRING("UP", get_interface_status_str(&wire_up));
    TEST_ASSERT_EQUAL_STRING("DOWN", get_interface_status_str(&wire_down));
    TEST_ASSERT_EQUAL_STRING("UP", get_interface_status_str(&wire_other));
    TEST_ASSERT_EQUAL_STRING("DOWN", get_interface_status_str(NULL));
}

void run_ipv6_tests(void) {
    RUN_TEST(test_should_show_ipv6_type_multicast);
    RUN_TEST(test_should_show_ipv6_type_link_local);
    RUN_TEST(test_should_show_ipv6_type_ula);
    RUN_TEST(test_should_show_ipv6_type_global);
    RUN_TEST(test_should_show_ipv6_type_unknown);
    RUN_TEST(test_should_show_ipv6_scope_unlimited);
    RUN_TEST(test_should_show_ipv6_scope_limited);
    RUN_TEST(test_get_ipv6_icon_multicast);
    RUN_TEST(test_get_ipv6_icon_link_local);
    RUN_TEST(test_get_ipv6_icon_ula);
    RUN_TEST(test_get_ipv6_icon_global);
    RUN_TEST(test_get_ipv6_icon_null);
    RUN_TEST(test_get_ipv6_icon_unknown_type);
    RUN_TEST(test_get_ipv6_icon_color_multicast);
    RUN_TEST(test_get_ipv6_icon_color_link_local);
    RUN_TEST(test_get_ipv6_icon_color_ula);
    RUN_TEST(test_get_ipv6_icon_color_global);
    RUN_TEST(test_get_ipv6_icon_color_null);
    RUN_TEST(test_get_ipv4_icon);
    RUN_TEST(test_get_ipv4_icon_color);
    RUN_TEST(test_interface_status_str);
}

void test_ipv6(void) {
    run_ipv6_tests();
}