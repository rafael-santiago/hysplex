/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef HYSPLEX_H
#define HYSPLEX_H 1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

struct hysplex_stat {
    char *func_name;
    long double exec_time_s;
    int score;
};

typedef void (*hysplex_meta_func)();

struct hysplex_eval_group {
    char *name;
    hysplex_meta_func func;
};

static FILE *hysplex_stdout = NULL;

static char **hysplex_argv = NULL;

static int hysplex_argc = 0;

#define HYSPLEX_FUNCTION_GROUP_BEGIN(name) struct hysplex_eval_group name[] = {

#define HYSPLEX_REGISTER_FUNCTION(func) { #func, (hysplex_meta_func)func }

#define HYSPLEX_FUNCTION_GROUP_END };

#define HYSPLEX_FUNCTION_GROUP_SIZE(name) sizeof(name) / sizeof(name[0])

#define HYSPLEX_SNPRINTF(buf, buf_size, ...) {\
    if (buf != NULL) {\
        snprintf(buf, buf_size, __VA_ARGS__);\
        buf += strlen(buf);\
        buf_size -= strlen(buf);\
    }\
}

#define HYSPLEX_BANNER {\
    fprintf(hysplex_stdout, "_     _ __   __  _____   _____          ______ _     _\n"\
                            "|_____|   \\_/   |_____  |_____] |      |______  \\___/ \n"\
                            "|     |    |     _____| |       |_____ |______ _/   \\_\n\n");\
}

#define HYSPLEX_ERROR(err) {\
    fprintf(stderr, "hysplex error: "err"\n");\
}

#define HYSPLEX_INFO(info) {\
    fprintf(hysplex_stdout, "hysplex info: "info);\
}

#define HYSPLEX_WARN(warn) {\
    fprintf(hysplex_stdout, "hysplex warn: "warn);\
}

#define HYSPLEX_PROGRESS(done, from) {\
    fprintf(hysplex_stdout, "\r                                                \r"\
                            "hysplex info: %.1f%% completed.", ((float)done / (float)from) * 100);\
    fflush(hysplex_stdout);\
}

#define HYSPLEX_DO_EVAL_PAIR(iter_nr, warm_up, wi, out, out_size, pre_run_stmt, post_run_stmt, func0, func1, ...) {\
        struct hysplex_stat hs[2] = { { #func0, 0, 0 }, { #func1, 0, 0 } };\
        if (warm_up) {\
            HYSPLEX_INFO("cache warming up... wait...\n");\
            for (size_t i = 0; i < iter_nr; i++) {\
                pre_run_stmt;\
                func0(__VA_ARGS__);\
                post_run_stmt;\
                pre_run_stmt;\
                func1(__VA_ARGS__);\
                post_run_stmt;\
                HYSPLEX_PROGRESS(i, iter_nr);\
            }\
        }\
        fprintf(hysplex_stdout, "\r                                             \r"\
                                "hysplex info: done.\n\n");\
        fprintf(hysplex_stdout, "hysplex info: '%s' vs. '%s'... wait...\n", #func0, #func1);\
        for (size_t i = 0; i < iter_nr; i++) {\
            clock_t start, end;\
            pre_run_stmt;\
            start = clock();\
            func0(__VA_ARGS__);\
            end = clock();\
            post_run_stmt;\
            long double func0_exec_time = ((long double)(end - start)) / ((long double)CLOCKS_PER_SEC);\
            hs[0].exec_time_s += func0_exec_time;\
            pre_run_stmt;\
            start = clock();\
            func1(__VA_ARGS__);\
            end = clock();\
            post_run_stmt;\
            long double func1_exec_time = ((long double)(end - start)) / ((long double)CLOCKS_PER_SEC);\
            hs[1].exec_time_s += func1_exec_time;\
            if (func0_exec_time < func1_exec_time) {\
                hs[0].score += 1;\
            } else if (func1_exec_time < func0_exec_time) {\
                hs[1].score += 1;\
            } else {\
                hs[0].score += 1;\
                hs[1].score += 1;\
            }\
            HYSPLEX_PROGRESS(i, iter_nr);\
        }\
        fprintf(hysplex_stdout, "\r                                             \r"\
                                "hysplex info: done.\n\n");\
        if ((wi = hysplex_get_winner_function(out, out_size, hs, 2, iter_nr, 1)) == -1) {\
            HYSPLEX_WARN("The evaluated performances are statistically equal.\n"\
                         "              In fact you must not taking into consideration performance to pick one.\n")\
            exit(1);\
        }\
}

#define HYSPLEX_DO_EVAL_GROUP(iter_nr, warm_up, pre_run_stmt, post_run_stmt, group, group_nr, ...) {\
    ssize_t lg = 0;\
    FILE *user_out = hysplex_stdout;\
    const char *out_option = hysplex_get_option("out", NULL);\
    if (out_option != NULL) {\
        if ((user_out = fopen(out_option, "a")) == NULL) {\
            user_out = hysplex_stdout;\
        }\
    }\
    for (ssize_t g = 1; g < group_nr; g++) {\
        struct hysplex_stat hs[2];\
        ssize_t wi[2];\
        char out[4096];\
        for (size_t r = 0; r < 2; r++) {\
            hysplex_meta_func f0, f1;\
            if (r == 0) {\
                hs[0].func_name = group[lg].name;\
                hs[1].func_name = group[g].name;\
                f0 = group[lg].func;\
                f1 = group[g].func;\
            } else {\
                hs[0].func_name = group[g].name;\
                hs[1].func_name = group[lg].name;\
                f0 = group[g].func;\
                f1 = group[lg].func;\
            }\
            hs[0].score = hs[1].score = 0;\
            hs[0].exec_time_s = hs[1].exec_time_s = 0;\
            if (warm_up) {\
                HYSPLEX_INFO("cache warming up... wait...\n");\
                for (size_t i = 0; i < iter_nr; i++) {\
                    pre_run_stmt;\
                    f0(__VA_ARGS__);\
                    post_run_stmt;\
                    pre_run_stmt;\
                    f1(__VA_ARGS__);\
                    post_run_stmt;\
                    HYSPLEX_PROGRESS(i, iter_nr);\
                }\
            }\
            fprintf(hysplex_stdout, "\r                                             \r"\
                                    "hysplex info: done.\n\n");\
            if (r == 0) {\
                fprintf(hysplex_stdout, "hysplex info: '%s' vs. '%s'... wait...\n", group[lg].name, group[g].name);\
            } else {\
                fprintf(hysplex_stdout, "hysplex info: '%s' vs. '%s... wait...\n", group[g].name, group[lg].name);\
            }\
            for (size_t i = 0; i < iter_nr; i++) {\
                clock_t start, end;\
                pre_run_stmt;\
                start = clock();\
                f0(__VA_ARGS__);\
                end = clock();\
                post_run_stmt;\
                long double f0_exec_time = ((long double)(end - start)) / ((long double)CLOCKS_PER_SEC);\
                hs[0].exec_time_s += f0_exec_time;\
                pre_run_stmt;\
                start = clock();\
                f1(__VA_ARGS__);\
                end = clock();\
                post_run_stmt;\
                long double f1_exec_time = ((long double)(end - start)) / ((long double)CLOCKS_PER_SEC);\
                hs[1].exec_time_s += f1_exec_time;\
                if (f0_exec_time < f1_exec_time) {\
                    hs[0].score += 1;\
                } else if (f1_exec_time < f0_exec_time) {\
                    hs[1].score += 1;\
                } else {\
                    hs[0].score += 1;\
                    hs[1].score += 1;\
                }\
                HYSPLEX_PROGRESS(i, iter_nr);\
            }\
            fprintf(hysplex_stdout, "\r                                             \r"\
                                "hysplex info: done.\n\n");\
            if ((wi[r] = hysplex_get_winner_function((r == 0) ? out : NULL, (r == 0) ? sizeof(out) - 1 : 0, hs, 2, iter_nr, ((g + 1) == group_nr))) == -1) {\
                HYSPLEX_WARN("The evaluated performances are statistically equal.\n"\
                             "              In fact you must not taking into consideration performance to pick one.\n")\
                exit(1);\
            }\
        }\
        if (wi[0] == wi[1]) {\
            if (user_out != hysplex_stdout) {\
                hysplex_stdout = user_out;\
            }\
            HYSPLEX_WARN("The evaluated performances are inconclusive. They are almost the same in performance. A bias was detected.\n"\
                         "              The second function seems to take advantage from the first, probably due to CPU cache.\n"\
                         "              You should take into consideration other aspects than performance to pick one. It is up to you!\n")\
            exit(1);\
        }\
        if (wi[0] == 1) {\
            lg = g;\
        }\
        fprintf(user_out, "%s\n", out);\
    }\
    if (user_out != hysplex_stdout) {\
        fclose(user_out);\
    }\
}

#define HYSPLEX_MAIN_BEGIN\
    int main(int argc, char **argv) {\
        hysplex_stdout = stdout;\
        hysplex_set_argc_argv(argc, argv);\
        if (!hysplex_validate_user_options()) {\
            exit(1);\
        }

#define HYSPLEX_MAIN_END\
    return 0;\
}

#define HYSPLEX_EVAL_PAIR(iter_nr, warm_up, pre_run_stmt, post_run_stmt, func0, func1, ...)\
    HYSPLEX_MAIN_BEGIN\
        HYSPLEX_BANNER\
        ssize_t wi[2];\
        char out[4096];\
        size_t curr_iter_nr = iter_nr;\
        const char *iterations = hysplex_get_option("iterations", NULL);\
        if (iterations != NULL) {\
            curr_iter_nr = strtoul(iterations, NULL, 10);\
        }\
        HYSPLEX_DO_EVAL_PAIR(curr_iter_nr, warm_up, wi[0], out, sizeof(out) - 1,\
                             pre_run_stmt, post_run_stmt, func0, func1, __VA_ARGS__)\
        HYSPLEX_DO_EVAL_PAIR(iter_nr, warm_up, wi[1], NULL, 0,\
                             pre_run_stmt, post_run_stmt, func1, func0, __VA_ARGS__)\
        const char *out_option = hysplex_get_option("out", NULL);\
        FILE *user_out = hysplex_stdout;\
        if (out_option != NULL) {\
            if ((user_out = fopen(out_option, "a")) == NULL) {\
                user_out = hysplex_stdout;\
            }\
        }\
        if (wi[0] != wi[1]) {\
            fprintf(user_out, "%s", out);\
        } else {\
            FILE *temp = NULL;\
            if (user_out != hysplex_stdout) {\
                temp = hysplex_stdout;\
                hysplex_stdout = user_out;\
            }\
            HYSPLEX_WARN("The evaluated performances are inconclusive. They are almost the same in performance. A bias was detected.\n"\
                         "              The second function seems to take advantage from the first, probably due to CPU cache.\n"\
                         "              You should take into consideration other aspects than performance to pick one. It is up to you!\n")\
            if (temp != NULL) {\
                hysplex_stdout = temp;\
            }\
            exit(1);\
        }\
        if (user_out != hysplex_stdout) {\
            fclose(user_out);\
        }\
HYSPLEX_MAIN_END

#define HYSPLEX_EVAL_GROUP(iter_nr, warm_up, pre_run_stmt, post_run_stmt, group, ...)\
    HYSPLEX_MAIN_BEGIN\
        HYSPLEX_BANNER\
        size_t curr_iter_nr = iter_nr;\
        const char *iterations = hysplex_get_option("iterations", NULL);\
        if (iterations != NULL) {\
            curr_iter_nr = strtoul(iterations, NULL, 10);\
        }\
        HYSPLEX_DO_EVAL_GROUP(curr_iter_nr, warm_up, pre_run_stmt, post_run_stmt, group,\
                              HYSPLEX_FUNCTION_GROUP_SIZE(group), __VA_ARGS__)\
HYSPLEX_MAIN_END

ssize_t hysplex_get_winner_function(char *buf, const size_t buf_size, struct hysplex_stat *hs, const size_t hs_nr,
                                    const size_t iter_nr, const int is_final);

void hysplex_set_argc_argv(const int argc, char **argv);

const char *hysplex_get_option(const char *option, const char *default_option);

int hysplex_get_bool_option(const char *option, const int default_option);

int hysplex_is_valid_number(const char *number, const size_t number_size);

int hysplex_validate_user_options(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HYSPLEX_H */
