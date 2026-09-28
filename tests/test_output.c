#include "unity.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FILE *captured_stdout;
static char capture_buffer[8192];

void capture_start(void) {
    captured_stdout = stdout;
    fflush(stdout);
    freopen("/tmp/test_capture.txt", "w", stdout);
    setvbuf(stdout, capture_buffer, _IOFBF, sizeof(capture_buffer));
}

const char *capture_end(void) {
    fflush(stdout);
    stdout = captured_stdout;
    FILE *f = fopen("/tmp/test_capture.txt", "r");
    if (f) {
        size_t n = fread(capture_buffer, 1, sizeof(capture_buffer) - 1, f);
        capture_buffer[n] = '\0';
        fclose(f);
        return capture_buffer;
    }
    return "";
}

void assert_output_contains(const char *expected) {
    const char *actual = capture_end();
    if (strstr(actual, expected) == NULL) {
        TEST_FAIL_MESSAGE(actual);
    }
}

void assert_output_equals(const char *expected) {
    const char *actual = capture_end();
    TEST_ASSERT_EQUAL_STRING(expected, actual);
}

Network *create_test_network(const char *iface, float link, float level, float noise) {
    Network *net = network_init();
    if (!net) return NULL;
    
    strncpy(net->wire->name, iface, 63);
    net->wire->name[63] = '\0';
    strcpy(net->wire->status, "0000");
    net->wire->link = link;
    net->wire->level = level;
    net->wire->noise = noise;
    
    IPv4 ipv4 = { .address = "192.168.1.100" };
    vec_push_addresses_ipv4(net->addresses_ipv4, ipv4);
    
    IPv6 ipv6 = { .address = "fe80::1", .type = "link-local", .scope_id = 1 };
    vec_push_addresses_ipv6(net->addresses_ipv6, ipv6);
    
    IPv6 ipv6_global = { .address = "2001:db8::1", .type = "global", .scope_id = 0 };
    vec_push_addresses_ipv6(net->addresses_ipv6, ipv6_global);
    
    return net;
}

void test_json_output_basic(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);
    
    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();
    
    TEST_ASSERT_TRUE(strstr(output, "\"networks\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"interface\": \"wlan0\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"ipv4\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"ipv6\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "192.168.1.100") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "fe80::1") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "2001:db8::1") != NULL);
    
    vec_networks_free(nets);
}

void test_json_output_signal_fields(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);
    
    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();
    
    TEST_ASSERT_TRUE(strstr(output, "\"quality\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"level_dbm\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"link\"") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "\"noise_dbm\"") != NULL);
    
    vec_networks_free(nets);
}

void test_json_output_multiple_networks(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net1 = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    Network *net2 = create_test_network("wlan1", 70.0f, -45.0f, -88.0f);
    vec_networks_push(nets, net1);
    vec_networks_push(nets, net2);
    free(net1);
    free(net2);
    
    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();
    
    int count = 0;
    const char *p = output;
    while ((p = strstr(p, "\"interface\"")) != NULL) {
        count++;
        p++;
    }
    TEST_ASSERT_EQUAL(2, count);
    
    vec_networks_free(nets);
}

void test_json_output_empty_networks(void) {
    VecNetwork *nets = vec_networks_init();
    
    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();
    
    TEST_ASSERT_TRUE(strstr(output, "\"networks\": []") != NULL);
    
    // Manually free to avoid ASan leak detection issue
    free(nets->data);
    free(nets);
    nets = NULL;
}

void test_json_output_null_networks(void) {
    capture_start();
    vec_networks_json_output(NULL);
    const char *output = capture_end();
    
    TEST_ASSERT_TRUE(strlen(output) == 0 || strstr(output, "{}") != NULL);
}

void test_terminal_output_verbose_style(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);

    capture_start();
    vec_networks_terminal_output(nets);
    const char *output = capture_end();

    TEST_ASSERT_TRUE(strstr(output, "wlan0") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "192.168.1.100") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "fe80::1") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "2001:db8::1") != NULL);
#if TERMINAL_STYLE == 1
    TEST_ASSERT_TRUE(strstr(output, "UP") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "+") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "|") != NULL);
#elif TERMINAL_STYLE == 3
    /* detailed: no boxes, ssid fallback (create_test_network leaves ssid empty) */
    TEST_ASSERT_TRUE(strstr(output, "noname") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "ip-addresses:") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "+") == NULL);
#endif

    vec_networks_free(nets);
}

void test_terminal_output_empty(void) {
    VecNetwork *nets = vec_networks_init();
    
    capture_start();
    vec_networks_terminal_output(nets);
    const char *output = capture_end();
    
    TEST_ASSERT_EQUAL_STRING("", output);
    
    vec_networks_free(nets);
}

void test_terminal_output_null(void) {
    capture_start();
    vec_networks_terminal_output(NULL);
    const char *output = capture_end();
    
    TEST_ASSERT_EQUAL_STRING("", output);
}

void test_terminal_output_ipv6_filtering(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);
    
    capture_start();
    vec_networks_terminal_output(nets);
    const char *output = capture_end();
    
    TEST_ASSERT_TRUE(strstr(output, "link-local") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "global") != NULL);
    
    vec_networks_free(nets);
}

void test_json_output_ssid_field(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    strcpy(net->wire->ssid, "MyHome");
    vec_networks_push(nets, net);
    free(net);

    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();

    TEST_ASSERT_TRUE(strstr(output, "\"ssid\": \"MyHome\"") != NULL);

    vec_networks_free(nets);
}

void test_json_output_ssid_fallback(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);

    capture_start();
    vec_networks_json_output(nets);
    const char *output = capture_end();

    TEST_ASSERT_TRUE(strstr(output, "\"ssid\": \"noname\"") != NULL);

    vec_networks_free(nets);
}

void test_short_output_basic(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    strcpy(net->wire->ssid, "MyHome");
    vec_networks_push(nets, net);
    free(net);

    capture_start();
    vec_networks_short_output(nets);
    const char *output = capture_end();

    TEST_ASSERT_TRUE(strstr(output, "MyHome") != NULL);
    TEST_ASSERT_TRUE(strstr(output, "+") == NULL);

    vec_networks_free(nets);
}

void test_short_output_fallback_and_guards(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = create_test_network("wlan0", 50.0f, -55.0f, -95.0f);
    vec_networks_push(nets, net);
    free(net);

    capture_start();
    vec_networks_short_output(nets);
    const char *output = capture_end();

    TEST_ASSERT_TRUE(strstr(output, "noname") != NULL);

    vec_networks_free(nets);
}

void test_short_output_null_guard(void) {
    capture_start();
    vec_networks_short_output(NULL);
    TEST_ASSERT_EQUAL_STRING("", capture_end());
}

void run_output_tests(void) {
    RUN_TEST(test_json_output_basic);
    RUN_TEST(test_json_output_signal_fields);
    RUN_TEST(test_json_output_multiple_networks);
    RUN_TEST(test_json_output_null_networks);
    RUN_TEST(test_json_output_ssid_field);
    RUN_TEST(test_json_output_ssid_fallback);
    RUN_TEST(test_terminal_output_verbose_style);
    RUN_TEST(test_terminal_output_empty);
    RUN_TEST(test_terminal_output_null);
    RUN_TEST(test_terminal_output_ipv6_filtering);
    RUN_TEST(test_short_output_basic);
    RUN_TEST(test_short_output_fallback_and_guards);
    RUN_TEST(test_short_output_null_guard);
}

void test_output(void) {
    run_output_tests();
}