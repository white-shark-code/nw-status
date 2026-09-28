#include "unity.h"
#include "utils.h"
#include <string.h>

void test_network_init(void) {
    Network *net = network_init();
    TEST_ASSERT_NOT_NULL(net);
    TEST_ASSERT_NOT_NULL(net->wire);
    TEST_ASSERT_NOT_NULL(net->addresses_ipv4);
    TEST_ASSERT_NOT_NULL(net->addresses_ipv6);
    TEST_ASSERT_EQUAL(0, net->addresses_ipv4->size);
    TEST_ASSERT_EQUAL(0, net->addresses_ipv6->size);
    network_free(net);
    free(net);
}

void test_network_init_with_capacity(void) {
    Network *net = network_init_with_capacity(16);
    TEST_ASSERT_NOT_NULL(net);
    TEST_ASSERT_NOT_NULL(net->wire);
    TEST_ASSERT_NOT_NULL(net->addresses_ipv4);
    TEST_ASSERT_NOT_NULL(net->addresses_ipv6);
    TEST_ASSERT_GREATER_OR_EQUAL(16, net->addresses_ipv4->capacity);
    TEST_ASSERT_GREATER_OR_EQUAL(16, net->addresses_ipv6->capacity);
    network_free(net);
    free(net);
}

void test_network_init_zero_capacity(void) {
    Network *net = network_init_with_capacity(0);
    TEST_ASSERT_NOT_NULL(net);
    network_free(net);
    free(net);
}

void test_network_free_null(void) {
    network_free(NULL);
    TEST_PASS();
}

void test_network_free_valid(void) {
    Network *net = network_init();
    TEST_ASSERT_NOT_NULL(net);
    network_free(net);
    free(net);
    TEST_PASS();
}

void test_vec_networks_init(void) {
    VecNetwork *nets = vec_networks_init();
    TEST_ASSERT_NOT_NULL(nets);
    TEST_ASSERT_EQUAL(0, nets->size);
    TEST_ASSERT_EQUAL(0, nets->capacity);
    TEST_ASSERT_NULL(nets->data);
    vec_networks_free(nets);
}

void test_vec_networks_push_first(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net = network_init();
    
    TEST_ASSERT_EQUAL(0, vec_networks_push(nets, net));
    TEST_ASSERT_EQUAL(1, nets->size);
    TEST_ASSERT_EQUAL(8, nets->capacity);
    TEST_ASSERT_NOT_NULL(nets->data);
    
    free(net);
    vec_networks_free(nets);
}

void test_vec_networks_push_multiple(void) {
    VecNetwork *nets = vec_networks_init();
    
    for (int i = 0; i < 10; i++) {
        Network *net = network_init();
        TEST_ASSERT_EQUAL(0, vec_networks_push(nets, net));
        free(net);
    }
    
    TEST_ASSERT_EQUAL(10, nets->size);
    TEST_ASSERT_GREATER_OR_EQUAL(10, nets->capacity);
    
    vec_networks_free(nets);
}

void test_vec_networks_push_null_networks(void) {
    TEST_ASSERT_EQUAL(-1, vec_networks_push(NULL, NULL));
}

void test_vec_networks_push_null_network(void) {
    VecNetwork *nets = vec_networks_init();
    TEST_ASSERT_EQUAL(-1, vec_networks_push(nets, NULL));
    vec_networks_free(nets);
}

void test_vec_networks_free_null(void) {
    vec_networks_free(NULL);
    TEST_PASS();
}

void test_vec_networks_free_empty(void) {
    VecNetwork *nets = vec_networks_init();
    vec_networks_free(nets);
    TEST_PASS();
}

void test_vec_networks_free_with_data(void) {
    VecNetwork *nets = vec_networks_init();
    for (int i = 0; i < 5; i++) {
        Network *net = network_init();
        vec_networks_push(nets, net);
        free(net);
    }
    vec_networks_free(nets);
    TEST_PASS();
}

void test_prepair_networks_using_wireless_null(void) {
    VecNetwork *nets = prepair_networks_using_wireless(NULL);
    TEST_ASSERT_NULL(nets);
}

void test_prepair_networks_using_wireless_empty(void) {
    VecWirelesses *wires = vec_init_wirelesses();
    VecNetwork *nets = prepair_networks_using_wireless(wires);
    TEST_ASSERT_NULL(nets);
    vec_free_wirelesses(wires);
}

void test_prepair_networks_using_wireless_valid(void) {
    VecWirelesses *wires = vec_init_wirelesses_with_capacity(2);
    Wireless w1 = { .name = "wlan0", .status = "0000", .link = 45.0f, .level = -55.0f, .noise = -95.0f };
    Wireless w2 = { .name = "wlan1", .status = "0000", .link = 72.0f, .level = -42.0f, .noise = -88.0f };
    vec_push_wirelesses(wires, w1);
    vec_push_wirelesses(wires, w2);
    
    VecNetwork *nets = prepair_networks_using_wireless(wires);
    
    TEST_ASSERT_NOT_NULL(nets);
    TEST_ASSERT_EQUAL(2, nets->size);
    TEST_ASSERT_EQUAL_STRING("wlan0", nets->data[0].wire->name);
    TEST_ASSERT_EQUAL_STRING("wlan1", nets->data[1].wire->name);
    
    vec_networks_free(nets);
    vec_free_wirelesses(wires);
}

void test_check_exist_wire_found(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net1 = network_init();
    strcpy(net1->wire->name, "wlan0");
    vec_networks_push(nets, net1);
    free(net1);
    
    struct ifaddrs ifa = { .ifa_name = "wlan0" };
    ssize_t idx = get_index_wire(ifa.ifa_name, nets);
    
    TEST_ASSERT_EQUAL(0, idx);
    vec_networks_free(nets);
}

void test_check_exist_wire_not_found(void) {
    VecNetwork *nets = vec_networks_init();
    Network *net1 = network_init();
    strcpy(net1->wire->name, "wlan0");
    vec_networks_push(nets, net1);
    free(net1);
    
    struct ifaddrs ifa = { .ifa_name = "eth0" };
    ssize_t idx = get_index_wire(ifa.ifa_name, nets);
    
    TEST_ASSERT_EQUAL(-1, idx);
    vec_networks_free(nets);
}

void test_check_exist_wire_empty_networks(void) {
    VecNetwork *nets = vec_networks_init();
    struct ifaddrs ifa = { .ifa_name = "wlan0" };
    ssize_t idx = get_index_wire(ifa.ifa_name, nets);
    TEST_ASSERT_EQUAL(-1, idx);
    vec_networks_free(nets);
}

void run_network_tests(void) {
    RUN_TEST(test_network_init);
    RUN_TEST(test_network_init_with_capacity);
    RUN_TEST(test_network_init_zero_capacity);
    RUN_TEST(test_network_free_null);
    RUN_TEST(test_network_free_valid);
    RUN_TEST(test_vec_networks_init);
    RUN_TEST(test_vec_networks_push_first);
    RUN_TEST(test_vec_networks_push_multiple);
    RUN_TEST(test_vec_networks_push_null_networks);
    RUN_TEST(test_vec_networks_push_null_network);
    RUN_TEST(test_vec_networks_free_null);
    RUN_TEST(test_vec_networks_free_empty);
    RUN_TEST(test_vec_networks_free_with_data);
    RUN_TEST(test_prepair_networks_using_wireless_null);
    RUN_TEST(test_prepair_networks_using_wireless_empty);
    RUN_TEST(test_prepair_networks_using_wireless_valid);
    RUN_TEST(test_check_exist_wire_found);
    RUN_TEST(test_check_exist_wire_not_found);
    RUN_TEST(test_check_exist_wire_empty_networks);
}

void test_network(void) {
    run_network_tests();
}