/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <cutest.h>
#include <hysplex.h>
#include <string.h>
#include <sys/stat.h>

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

static char invalid_certainty_perc[] = "--certainty-perc=5";
static char valid_certainty_perc90[] = "--certainty-perc=90";
static char valid_certainty_perc95[] = "--certainty-perc=95";
static char valid_certainty_perc97[] = "--certainty-perc=97";
static char valid_certainty_perc99[] = "--certainty-perc=99";
static char valid_certainty_perc100[] = "--certainty-perc=100";
static char invalid_iterations_negative[] = "--iterations=-382";
static char invalid_iterations_zero[] = "--iterations=0000000000";
static char valid_iterations[] = "--iterations=1234567890";

static char out[] = "--out=hysplex.log";

static char *poke_tests_argv[] = {
    app_path,
    out,
};

static char *invalid_certainty_perc_argv[] = {
    app_path,
    invalid_certainty_perc,
};

static char *valid_certainty_perc90_argv[] = {
    app_path,
    valid_certainty_perc90,
};

static char *valid_certainty_perc95_argv[] = {
    app_path,
    valid_certainty_perc95,
};

static char *valid_certainty_perc97_argv[] = {
    app_path,
    valid_certainty_perc97,
};

static char *valid_certainty_perc99_argv[] = {
    app_path,
    valid_certainty_perc99,
};

static char *valid_certainty_perc100_argv[] = {
    app_path,
    valid_certainty_perc100,
};

static char *invalid_iterations_negative_argv[] = {
    app_path,
    invalid_iterations_negative,
};

static char *invalid_iterations_zero_argv[] = {
    app_path,
    invalid_iterations_zero,
};

static char *valid_iterations_argv[] = {
    app_path,
    valid_iterations,
};

CUTE_DECLARE_TEST_CASE(hysplex_get_option_tests);
CUTE_DECLARE_TEST_CASE(hysplex_get_bool_option_tests);
CUTE_DECLARE_TEST_CASE(hysplex_is_valid_number_tests);
CUTE_DECLARE_TEST_CASE(hysplex_validate_user_options_tests);
CUTE_DECLARE_TEST_CASE(hysplex_get_winner_function_tests);
CUTE_DECLARE_TEST_CASE(hysplex_poke_tests);

CUTE_DECLARE_TEST_CASE(hysplex_tests);

CUTE_TEST_CASE(hysplex_tests)
    CUTE_RUN_TEST(hysplex_get_option_tests);
    CUTE_RUN_TEST(hysplex_get_bool_option_tests);
    CUTE_RUN_TEST(hysplex_is_valid_number_tests);
    CUTE_RUN_TEST(hysplex_validate_user_options_tests);
    CUTE_RUN_TEST(hysplex_get_winner_function_tests);
    CUTE_RUN_TEST(hysplex_poke_tests);
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
    struct test_ctx {
        char **argv;
        int argc;
        int expected;
    } test_ctx[] = {
        { invalid_certainty_perc_argv, sizeof(invalid_certainty_perc_argv) / sizeof(invalid_certainty_perc_argv[0]), 0 },
        { valid_certainty_perc90_argv, sizeof(valid_certainty_perc90_argv) / sizeof(valid_certainty_perc90_argv[0]), 1 },
        { valid_certainty_perc95_argv, sizeof(valid_certainty_perc95_argv) / sizeof(valid_certainty_perc95_argv[0]), 1 },
        { valid_certainty_perc97_argv, sizeof(valid_certainty_perc97_argv) / sizeof(valid_certainty_perc97_argv[0]), 1 },
        { valid_certainty_perc99_argv, sizeof(valid_certainty_perc99_argv) / sizeof(valid_certainty_perc99_argv[0]), 1 },
        { valid_certainty_perc100_argv, sizeof(valid_certainty_perc100_argv) / sizeof(valid_certainty_perc100_argv[0]), 1 },
        { invalid_iterations_negative_argv, sizeof(invalid_iterations_negative_argv) /
                                                    sizeof(invalid_iterations_negative_argv[0]), 0 },
        { invalid_iterations_zero_argv, sizeof(invalid_iterations_zero_argv) / sizeof(invalid_iterations_zero_argv[0]), 0 },
        { valid_iterations_argv, sizeof(valid_iterations_argv) / sizeof(valid_iterations_argv[0]), 1 },
    }, *test, *test_end;
    test = &test_ctx[0];
    test_end = test + sizeof(test_ctx) / sizeof(test_ctx[0]);
    while (test != test_end) {
        hysplex_set_argc_argv(test->argc, test->argv);
        CUTE_ASSERT(hysplex_validate_user_options() == test->expected);
        test++;
    }
    hysplex_set_argc_argv(0, NULL);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_get_winner_function_tests)
    char buf[4096];
    struct hysplex_stat hs[2];

    hs[0].exec_time_s = 5;
    hs[0].score = 70;
    hs[0].func_name = "CARA";
    hs[1].exec_time_s = 10;
    hs[1].score = 30;
    hs[1].func_name = "COROA";

    ssize_t wi = hysplex_get_winner_function(buf, sizeof(buf) - 1, &hs[0], 2, 100, 0);

    CUTE_ASSERT(strstr(buf, "== Hysplex intermediate stats\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Hysplex final stats\n") == NULL);
    CUTE_ASSERT(strstr(buf, "== Functions 'CARA' and 'COROA' were executed 100 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Function 'CARA' has won 70 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Function 'COROA' has won 30 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Total of tied executions: 0.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Total of iterations with a winner: 100\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The average execution time of function 'CARA' was about 0.050000 secs.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The average execution time of function 'COROA' was about 0.100000 secs.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The winner function is 'CARA'.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Chi-square = 16.000 (certainty = 95%), 'CARA' is statistically faster than 'COROA'.") != NULL);

    hs[0].score += 20;
    hs[1].score += 20;

    wi = hysplex_get_winner_function(buf, sizeof(buf) - 1, &hs[0], 2, 100, 1);

    CUTE_ASSERT(strstr(buf, "== Hysplex final stats\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Hysplex intermediate stats\n") == NULL);
    CUTE_ASSERT(strstr(buf, "== Functions 'CARA' and 'COROA' were executed 100 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Function 'CARA' has won 90 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Function 'COROA' has won 50 time(s).\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Total of tied executions: 20.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Total of iterations with a winner: 80\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The average execution time of function 'CARA' was about 0.050000 secs.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The average execution time of function 'COROA' was about 0.100000 secs.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== The winner function is 'CARA'.\n") != NULL);
    CUTE_ASSERT(strstr(buf, "== Chi-square = 25.000 (certainty = 95%), 'CARA' is statistically faster than 'COROA'.") != NULL);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(hysplex_poke_tests)
    struct test_ctx {
        const char *cmd;
        const char *expected;
        int success;
    } test_ctx[] = {
#if !defined(_WIN32)
        { "../../samples/eval-pair --out=hysplex.log", "'plain_sum' is statistically faster than 'delayed_sum'.", 1 },
        { "../../samples/eval-pair-inconclusive --out=hysplex.log", "The evaluated performances are inconclusive.", 0 },
        { "../../samples/eval-group --out=hysplex.log",
          "'stat_based_file_exists' is statistically faster than 'open_based_file_exists'.", 1 },
#else
        { "..\\..\\samples\\eval-pair.exe --out=hysplex.log", "'plain_sum' is statistically faster than 'delayed_sum'.", 1 },
        { "..\\..\\samples\\eval-pair-inconclusive.exe --out=hysplex.log",
          "The evaluated performances are inconclusive.", 0 },
        { "..\\..\\samples\\eval-group.exe --out=hysplex.log",
          "'stat_based_file_exists' is statistically faster than 'open_based_file_exists'.", 1 },
#endif
    }, *test, *test_end;
    int argc = sizeof(poke_tests_argv) / sizeof(poke_tests_argv[0]);
    test = &test_ctx[0];
    test_end = test + sizeof(test_ctx) / sizeof(test_ctx[0]);
    hysplex_set_argc_argv(argc, poke_tests_argv);
    while (test != test_end) {
        remove("hysplex.log");
        if (test->success) {
            CUTE_ASSERT(system(test->cmd) == 0);
        } else {
            CUTE_ASSERT(system(test->cmd) != 0);
        }
        struct stat st;
        CUTE_ASSERT(stat("hysplex.log", &st) == 0);
        FILE *fp = fopen("hysplex.log", "r");
        CUTE_ASSERT(fp != NULL);
        char *buf = (char *) calloc(sizeof(char), st.st_size + 1);
        CUTE_ASSERT(buf != NULL);
        fread(buf, 1, st.st_size, fp);
        fclose(fp);
        CUTE_ASSERT(strstr(buf, test->expected) != NULL);
        free(buf);
        test++;
    }
    remove("hysplex.log");
CUTE_TEST_CASE_END
