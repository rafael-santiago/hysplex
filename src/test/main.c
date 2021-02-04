#include <cutest.h>
#include <hysplex.h>
#include <string.h>

static char app_path[] = "./hysplex-tests";
static char option1[] = "--foo=bar";
static char option2[] = "--bar";
static char option3[] = "--foobar=baz";

static char *argv[] = {
    app_path,
    option1,
    option2,
    option3,
};

static int argc = sizeof(argv) / sizeof(argv[0]);

CUTE_DECLARE_TEST_CASE(hysplex_get_option_tests);
CUTE_DECLARE_TEST_CASE(hysplex_get_bool_option_tests);
CUTE_DECLARE_TEST_CASE(hysplex_is_valid_number_tests);
CUTE_DECLARE_TEST_CASE(hysplex_validate_user_options_tests);
CUTE_DECLARE_TEST_CASE(hysplex_get_winner_function_tests);

CUTE_DECLARE_TEST_CASE(hysplex_tests);

CUTE_TEST_CASE(hysplex_tests)
    CUTE_RUN_TEST(hysplex_get_option_tests);
    CUTE_RUN_TEST(hysplex_get_bool_option_tests);
    CUTE_RUN_TEST(hysplex_is_valid_number_tests);
    CUTE_RUN_TEST(hysplex_validate_user_options_tests);
    CUTE_RUN_TEST(hysplex_get_winner_function_tests);
CUTE_TEST_CASE_END

CUTE_MAIN(hysplex_tests)

CUTE_TEST_CASE(hysplex_get_option_tests)
    CUTE_ASSERT(hysplex_get_option("foo", NULL) == NULL);
    CUTE_ASSERT(hysplex_get_option("bar", NULL) == NULL);
    CUTE_ASSERT(hysplex_get_option("foobar", NULL) == NULL);
    struct test_ctx {
        const char *option;
        const char *default_value;
        const char *expected;
    } test_ctx[] = {
        { "foo", "oof", "bar" },
        { "bar", "rab", "rab" },
        { "foobar", "raboof", "baz" },
        { "unk", "knu", "knu" },
    }, *test, *test_end;
    test = &test_ctx[0];
    test_end = test + sizeof(test_ctx) / sizeof(test_ctx[0]);
    hysplex_set_argc_argv(argc, argv);
    while (test != test_end) {
        const char *data = hysplex_get_option(test->option, test->default_value);
        CUTE_ASSERT(data != NULL);
        CUTE_ASSERT(strcmp(data, test->expected) == 0);
        test++;
    }
    hysplex_set_argc_argv(0, NULL);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_get_bool_option_tests)
    CUTE_ASSERT(hysplex_get_bool_option("foo", 0) == 0);
    CUTE_ASSERT(hysplex_get_bool_option("bar", 0) == 0);
    CUTE_ASSERT(hysplex_get_bool_option("foobar", 0) == 0);
    struct test_ctx {
        const char *option;
        const int default_value;
        const int expected;
    } test_ctx[] = {
        { "foo", 1, 1 },
        { "bar", 0, 1 },
        { "foobar", 1, 1 },
        { "unk", 1, 1 },
    }, *test, *test_end;
    test = &test_ctx[0];
    test_end = test + sizeof(test_ctx) / sizeof(test_ctx[0]);
    hysplex_set_argc_argv(argc, argv);
    while (test != test_end) {
        CUTE_ASSERT(hysplex_get_bool_option(test->option, test->default_value) == test->expected);
        test++;
    }
    hysplex_set_argc_argv(0, NULL);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_is_valid_number_tests)
    CUTE_ASSERT(hysplex_is_valid_number(NULL, 10) == 0);
    CUTE_ASSERT(hysplex_is_valid_number("10", 0) == 0);
    CUTE_ASSERT(hysplex_is_valid_number("10", 1) == 1);
    CUTE_ASSERT(hysplex_is_valid_number("10", 2) == 1);
    struct test_ctx {
        const char *number;
        const int expected;
    } test_ctx[] = {
        { "1", 1 },
        { "-1", 0 },
        { "um", 0 },
        { ".1", 0 },
        { "01234567899876543210", 1 },
        { "0123456789987654321.0", 0 },
    }, *test, *test_end;
    test = &test_ctx[0];
    test_end = test + sizeof(test_ctx) / sizeof(test_ctx[0]);
    while (test != test_end) {
        CUTE_ASSERT(hysplex_is_valid_number(test->number, strlen(test->number)) == test->expected);
        test++;
    }
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_validate_user_options_tests)
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_get_winner_function_tests)
CUTE_TEST_CASE_END
