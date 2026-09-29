#include "unity.h"
#include "utils.h"
#include "config.h"
#include <string.h>

static const signal_level_t test_signal_levels[] = {
    {-70.0f, "LOW", "󰤯", "#ff0000"},
    {-50.0f, "MEDIUM", "󰤟", "#ffff00"},
    {-30.0f, "HIGH", "󰤨", "#00ff00"},
};
#define TEST_SIGNAL_LEVEL_COUNT (sizeof(test_signal_levels) / sizeof(test_signal_levels[0]))

const char *get_signal_quality_test(float noise) {
    for (size_t i = 0; i < TEST_SIGNAL_LEVEL_COUNT; i++) {
        if (noise < test_signal_levels[i].threshold) {
            return test_signal_levels[i].label;
        }
    }
    return test_signal_levels[TEST_SIGNAL_LEVEL_COUNT - 1].label;
}

const char *get_signal_icon_test(float noise) {
    for (size_t i = 0; i < TEST_SIGNAL_LEVEL_COUNT; i++) {
        if (noise < test_signal_levels[i].threshold) {
            return test_signal_levels[i].icon;
        }
    }
    return test_signal_levels[TEST_SIGNAL_LEVEL_COUNT - 1].icon;
}

const char *get_signal_color_test(float noise) {
    for (size_t i = 0; i < TEST_SIGNAL_LEVEL_COUNT; i++) {
        if (noise < test_signal_levels[i].threshold) {
            return test_signal_levels[i].color;
        }
    }
    return test_signal_levels[TEST_SIGNAL_LEVEL_COUNT - 1].color;
}

void test_get_signal_quality_low(void) {
    TEST_ASSERT_EQUAL_STRING("LOW", get_signal_quality_test(-80.0f));
    TEST_ASSERT_EQUAL_STRING("MEDIUM", get_signal_quality_test(-70.0f));
}

void test_get_signal_quality_medium(void) {
    TEST_ASSERT_EQUAL_STRING("MEDIUM", get_signal_quality_test(-69.0f));
    TEST_ASSERT_EQUAL_STRING("MEDIUM", get_signal_quality_test(-51.0f));
}

void test_get_signal_quality_high(void) {
    TEST_ASSERT_EQUAL_STRING("HIGH", get_signal_quality_test(-49.0f));
    TEST_ASSERT_EQUAL_STRING("HIGH", get_signal_quality_test(-30.0f));
    TEST_ASSERT_EQUAL_STRING("HIGH", get_signal_quality_test(-10.0f));
}

void test_get_signal_icon_low(void) {
    TEST_ASSERT_EQUAL_STRING("󰤯", get_signal_icon_test(-80.0f));
}

void test_get_signal_icon_medium(void) {
    TEST_ASSERT_EQUAL_STRING("󰤟", get_signal_icon_test(-60.0f));
}

void test_get_signal_icon_high(void) {
    TEST_ASSERT_EQUAL_STRING("󰤨", get_signal_icon_test(-20.0f));
}

void test_get_signal_color_low(void) {
    TEST_ASSERT_EQUAL_STRING("#ff0000", get_signal_color_test(-80.0f));
}

void test_get_signal_color_medium(void) {
    TEST_ASSERT_EQUAL_STRING("#ffff00", get_signal_color_test(-60.0f));
}

void test_get_signal_color_high(void) {
    TEST_ASSERT_EQUAL_STRING("#00ff00", get_signal_color_test(-20.0f));
}

static const link_icon_t test_link_icons[] = {
    { 70, "󰇧", "#00ff00" },
    { 40, "󰇨", "#ffff00" },
    { 0,  "󰇦", "#ff0000" },
};
#define TEST_LINK_ICON_COUNT (sizeof(test_link_icons) / sizeof(test_link_icons[0]))

const char *get_link_icon_test(int link) {
    for (size_t i = 0; i < TEST_LINK_ICON_COUNT; i++) {
        if (link >= test_link_icons[i].threshold) {
            return test_link_icons[i].icon;
        }
    }
    return DEFAULT_LINK_ICON;
}

const char *get_link_icon_color_test(int link) {
    for (size_t i = 0; i < TEST_LINK_ICON_COUNT; i++) {
        if (link >= test_link_icons[i].threshold) {
            return test_link_icons[i].color ? test_link_icons[i].color : DEFAULT_LINK_ICON_COLOR;
        }
    }
    return DEFAULT_LINK_ICON_COLOR;
}

void test_get_link_icon_good(void) {
    TEST_ASSERT_EQUAL_STRING("󰇧", get_link_icon_test(80));
    TEST_ASSERT_EQUAL_STRING("󰇧", get_link_icon_test(70));
}

void test_get_link_icon_medium(void) {
    TEST_ASSERT_EQUAL_STRING("󰇨", get_link_icon_test(69));
    TEST_ASSERT_EQUAL_STRING("󰇨", get_link_icon_test(40));
}

void test_get_link_icon_poor(void) {
    TEST_ASSERT_EQUAL_STRING("󰇦", get_link_icon_test(39));
    TEST_ASSERT_EQUAL_STRING("󰇦", get_link_icon_test(0));
}

void test_get_link_icon_color_good(void) {
    TEST_ASSERT_EQUAL_STRING("#00ff00", get_link_icon_color_test(80));
}

void test_get_link_icon_color_medium(void) {
    TEST_ASSERT_EQUAL_STRING("#ffff00", get_link_icon_color_test(50));
}

void test_get_link_icon_color_poor(void) {
    TEST_ASSERT_EQUAL_STRING("#ff0000", get_link_icon_color_test(20));
}

static const level_icon_t test_level_icons[] = {
    { -80.0f, "󰤯", "#ff0000" },
    { -60.0f, "󰤟", "#ffff00" },
    { -40.0f, "󰤢", "#00ff00" },
    { -30.0f, "󰤨", "#00ff00" },
};
#define TEST_LEVEL_ICON_COUNT (sizeof(test_level_icons) / sizeof(test_level_icons[0]))

const char *get_level_icon_test(float level) {
    for (size_t i = 0; i < TEST_LEVEL_ICON_COUNT; i++) {
        if (level >= test_level_icons[i].threshold) {
            return test_level_icons[i].icon;
        }
    }
    return DEFAULT_LEVEL_ICON;
}

const char *get_level_icon_color_test(float level) {
    for (size_t i = 0; i < TEST_LEVEL_ICON_COUNT; i++) {
        if (level >= test_level_icons[i].threshold) {
            return test_level_icons[i].color ? test_level_icons[i].color : DEFAULT_LEVEL_ICON_COLOR;
        }
    }
    return DEFAULT_LEVEL_ICON_COLOR;
}

void test_configured_icon_tables(void) {
    /* default config tables: link 50 -> medium, level -50 -> good */
    TEST_ASSERT_EQUAL_STRING("󰇨", get_link_icon(50));
    TEST_ASSERT_EQUAL_STRING("#ffff00", get_link_icon_color(50));
    TEST_ASSERT_EQUAL_STRING("󰤢", get_level_icon(-50.0f));
    TEST_ASSERT_EQUAL_STRING("#aaff00", get_level_icon_color(-50.0f));
    /* unknown interface still falls back to defaults */
    TEST_ASSERT_EQUAL_STRING(DEFAULT_INTERFACE_ICON, get_interface_icon("nope0"));
}

void run_signal_tests(void) {
    RUN_TEST(test_get_signal_quality_low);
    RUN_TEST(test_get_signal_quality_medium);
    RUN_TEST(test_get_signal_quality_high);
    RUN_TEST(test_get_signal_icon_low);
    RUN_TEST(test_get_signal_icon_medium);
    RUN_TEST(test_get_signal_icon_high);
    RUN_TEST(test_get_signal_color_low);
    RUN_TEST(test_get_signal_color_medium);
    RUN_TEST(test_get_signal_color_high);
    RUN_TEST(test_get_link_icon_good);
    RUN_TEST(test_get_link_icon_medium);
    RUN_TEST(test_get_link_icon_poor);
    RUN_TEST(test_get_link_icon_color_good);
    RUN_TEST(test_get_link_icon_color_medium);
    RUN_TEST(test_get_link_icon_color_poor);
    RUN_TEST(test_configured_icon_tables);
}

void test_signals(void) {
    run_signal_tests();
}