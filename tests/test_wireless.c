#include "unity.h"
#include "utils.h"
#include <string.h>

static const char *fixture_minimal = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlan0: 0000   70.  -35.  -256        0      0      0      0     49        0\n";

static const char *fixture_old_format = 
    "Inter-|sta| Quality | Discarded packets\n"
    "face |tus|link level noise| nwid crypt misc\n"
    "eth2: f0 15. 24. 4. 181 0 0\n";

static const char *fixture_pine64 = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlan0: 0000    0.  -256.  -256.     0     0    0    0     0       0\n"
    " wlan1: 0000   42.   -73.  -256.     0     0    0    0     0       0\n";

static const char *fixture_dual_band = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlp3s0: 0000   70.   -42.   -92.     0     0    0   47    18       2\n"
    " wlp4s0: 0000   48.   -67.   -90.     2     0   11   22     5       0\n";

static const char *fixture_not_associated = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlan0: 0001    0  -256  -256     0     0    0    0     0       0\n";

static const char *fixture_all_bad = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlan0: 00ff  23.  -78.  -89.   421    87  103  1409  562       37\n";

static const char *fixture_mixed_updates = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n"
    " wlan0: 0000   68.   -50.   -94     0     0    0    0     0       0\n";

static const char *fixture_empty = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n";

static const char *fixture_only_headers = 
    "Inter-| sta-| Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n";

void test_slice_char_valid(void) {
    VecChar *row = slice_char("hello", 5);
    TEST_ASSERT_NOT_NULL(row);
    TEST_ASSERT_EQUAL(5, row->size);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", row->data, 5);
    vec_free_char(row);
}

void test_slice_char_exact_length(void) {
    VecChar *row = slice_char("hello", 5);
    TEST_ASSERT_NOT_NULL(row);
    TEST_ASSERT_EQUAL(5, row->size);
    vec_free_char(row);
}

void test_slice_char_longer_string(void) {
    VecChar *row = slice_char("hello world", 5);
    TEST_ASSERT_NOT_NULL(row);
    TEST_ASSERT_EQUAL(5, row->size);
    TEST_ASSERT_EQUAL_STRING_LEN("hello", row->data, 5);
    vec_free_char(row);
}

void test_slice_char_null(void) {
    VecChar *row = slice_char(NULL, 5);
    TEST_ASSERT_NULL(row);
}

void test_slice_char_len_exceeds_string(void) {
    VecChar *row = slice_char("hi", 5);
    TEST_ASSERT_NULL(row);
}

void test_slice_char_zero_len(void) {
    VecChar *row = slice_char("hello", 0);
    TEST_ASSERT_NOT_NULL(row);
    TEST_ASSERT_EQUAL(0, row->size);
    TEST_ASSERT_EQUAL_STRING("", row->data);
    vec_free_char(row);
}

void test_get_wire_iface_valid(void) {
    Wireless w = get_wire_iface(" wlan0: 0000   70.  -35.  -256");
    TEST_ASSERT_EQUAL_STRING("wlan0", w.name);
    TEST_ASSERT_EQUAL_STRING("0000", w.status);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, w.link);
    TEST_ASSERT_EQUAL_FLOAT(-35.0f, w.level);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, w.noise);
}

void test_get_wire_iface_old_format(void) {
    Wireless w = get_wire_iface(" eth2: f0 15. 24. 4.");
    TEST_ASSERT_EQUAL_STRING("eth2", w.name);
    TEST_ASSERT_EQUAL_STRING("f0", w.status);
    TEST_ASSERT_EQUAL_FLOAT(15.0f, w.link);
    TEST_ASSERT_EQUAL_FLOAT(24.0f, w.level);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, w.noise);
}

void test_get_wire_iface_noise_without_dot(void) {
    Wireless w = get_wire_iface(" wlan0: 0000   65.   -45.  -256");
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, w.noise);
}

void test_get_wire_iface_zero_without_dot(void) {
    Wireless w = get_wire_iface(" wlan0: 0000    0     0     0");
    TEST_ASSERT_EQUAL_FLOAT(0.0f, w.link);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, w.level);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, w.noise);
}

void test_get_wire_iface_null(void) {
    Wireless w = get_wire_iface(NULL);
    TEST_ASSERT_EQUAL_STRING("", w.name);
}

void test_get_wire_iface_invalid_format(void) {
    Wireless w = get_wire_iface("invalid line");
    TEST_ASSERT_EQUAL_STRING("", w.name);
}

void test_get_wire_iface_empty_name(void) {
    Wireless w = get_wire_iface(" : 0000   45.  -55.  -95.");
    TEST_ASSERT_EQUAL_STRING("", w.name);
}

void test_get_wire_iface_status_hex(void) {
    Wireless w = get_wire_iface(" wlan0: 0001    0  -256  -256");
    TEST_ASSERT_EQUAL_STRING("0001", w.status);
}

void test_get_all_wire_ifaces_mock_file(void) {
    FILE *f = fmemopen((void*)fixture_minimal, strlen(fixture_minimal), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(1, wires->size);
    
    TEST_ASSERT_EQUAL_STRING("wlan0", wires->data[0].name);
    TEST_ASSERT_EQUAL_STRING("0000", wires->data[0].status);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-35.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[0].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_pine64(void) {
    FILE *f = fmemopen((void*)fixture_pine64, strlen(fixture_pine64), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(2, wires->size);
    
    TEST_ASSERT_EQUAL_STRING("wlan0", wires->data[0].name);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[0].noise);
    
    TEST_ASSERT_EQUAL_STRING("wlan1", wires->data[1].name);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, wires->data[1].link);
    TEST_ASSERT_EQUAL_FLOAT(-73.0f, wires->data[1].level);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[1].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_dual_band(void) {
    FILE *f = fmemopen((void*)fixture_dual_band, strlen(fixture_dual_band), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(2, wires->size);
    
    TEST_ASSERT_EQUAL_STRING("wlp3s0", wires->data[0].name);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-42.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-92.0f, wires->data[0].noise);
    
    TEST_ASSERT_EQUAL_STRING("wlp4s0", wires->data[1].name);
    TEST_ASSERT_EQUAL_FLOAT(48.0f, wires->data[1].link);
    TEST_ASSERT_EQUAL_FLOAT(-67.0f, wires->data[1].level);
    TEST_ASSERT_EQUAL_FLOAT(-90.0f, wires->data[1].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_not_associated(void) {
    FILE *f = fmemopen((void*)fixture_not_associated, strlen(fixture_not_associated), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(1, wires->size);
    TEST_ASSERT_EQUAL_STRING("wlan0", wires->data[0].name);
    TEST_ASSERT_EQUAL_STRING("0001", wires->data[0].status);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-256.0f, wires->data[0].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_all_bad(void) {
    FILE *f = fmemopen((void*)fixture_all_bad, strlen(fixture_all_bad), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(1, wires->size);
    TEST_ASSERT_EQUAL_STRING("wlan0", wires->data[0].name);
    TEST_ASSERT_EQUAL_STRING("00ff", wires->data[0].status);
    TEST_ASSERT_EQUAL_FLOAT(23.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-78.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-89.0f, wires->data[0].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_mixed_updates(void) {
    FILE *f = fmemopen((void*)fixture_mixed_updates, strlen(fixture_mixed_updates), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(1, wires->size);
    TEST_ASSERT_EQUAL_FLOAT(68.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(-50.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(-94.0f, wires->data[0].noise);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_empty(void) {
    FILE *f = fmemopen((void*)fixture_empty, strlen(fixture_empty), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(0, wires->size);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_only_headers(void) {
    FILE *f = fmemopen((void*)fixture_only_headers, strlen(fixture_only_headers), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(0, wires->size);
    
    vec_free_wirelesses(wires);
}

void test_get_all_wire_ifaces_old_format(void) {
    FILE *f = fmemopen((void*)fixture_old_format, strlen(fixture_old_format), "r");
    TEST_ASSERT_NOT_NULL(f);
    
    VecWirelesses *wires = get_all_wire_ifaces(f);
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(wires);
    TEST_ASSERT_EQUAL(1, wires->size);
    TEST_ASSERT_EQUAL_STRING("eth2", wires->data[0].name);
    TEST_ASSERT_EQUAL_STRING("f0", wires->data[0].status);
    TEST_ASSERT_EQUAL_FLOAT(15.0f, wires->data[0].link);
    TEST_ASSERT_EQUAL_FLOAT(24.0f, wires->data[0].level);
    TEST_ASSERT_EQUAL_FLOAT(4.0f, wires->data[0].noise);
    
    vec_free_wirelesses(wires);
}

void test_read_file_nonexistent(void) {
    FILE *f = read_file("/nonexistent/path/file");
    TEST_ASSERT_NULL(f);
}

void test_read_file_valid_path(void) {
    FILE *f = read_file("/proc/net/wireless");
    if (f) fclose(f);
    TEST_PASS();
}

void run_wireless_tests(void) {
    RUN_TEST(test_slice_char_valid);
    RUN_TEST(test_slice_char_exact_length);
    RUN_TEST(test_slice_char_longer_string);
    RUN_TEST(test_slice_char_null);
    RUN_TEST(test_slice_char_len_exceeds_string);
    RUN_TEST(test_slice_char_zero_len);
    RUN_TEST(test_get_wire_iface_valid);
    RUN_TEST(test_get_wire_iface_old_format);
    RUN_TEST(test_get_wire_iface_noise_without_dot);
    RUN_TEST(test_get_wire_iface_zero_without_dot);
    RUN_TEST(test_get_wire_iface_null);
    RUN_TEST(test_get_wire_iface_invalid_format);
    RUN_TEST(test_get_wire_iface_empty_name);
    RUN_TEST(test_get_wire_iface_status_hex);
    RUN_TEST(test_get_all_wire_ifaces_mock_file);
    RUN_TEST(test_get_all_wire_ifaces_pine64);
    RUN_TEST(test_get_all_wire_ifaces_dual_band);
    RUN_TEST(test_get_all_wire_ifaces_not_associated);
    RUN_TEST(test_get_all_wire_ifaces_all_bad);
    RUN_TEST(test_get_all_wire_ifaces_mixed_updates);
    RUN_TEST(test_get_all_wire_ifaces_empty);
    RUN_TEST(test_get_all_wire_ifaces_only_headers);
    RUN_TEST(test_get_all_wire_ifaces_old_format);
    RUN_TEST(test_read_file_nonexistent);
    RUN_TEST(test_read_file_valid_path);
}

void test_wireless(void) {
    run_wireless_tests();
}