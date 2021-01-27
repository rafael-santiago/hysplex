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
#include <string.h>
#include <stdio.h>
#include <time.h>

struct hysplex_stat {
    char *func_name;
    long double exec_time_s;
    int score;
};

typedef void (*hysplex_any_func)();

struct hysplex_eval_group {
    char *name;
    hysplex_any_func func;
};

#define HYSPLEX_FUNCTION_GROUP_BEGIN(name) struct hysplex_eval_group name[] = {

#define HYSPLEX_REGISTER_FUNCTION(func) { #func, (hysplex_any_func)func }

#define HYSPLEX_FUNCTION_GROUP_END }

#define HYSPLEX_FUNCTION_GROUP_SIZE(name) sizeof(name) / sizeof(name[0])

FILE *hysplex_stdout = NULL;

#define HYSPLEX_BANNER {\
    fprintf(hysplex_stdout, " _     _ __   __ _______  _____         _______ _     _\n"\
                            " |_____|   \_/   |______ |_____] |      |______  \___/ \n"\
                            " |     |    |    ______| |       |_____ |______ _/   \_\n");\
}

#define HYSPLEX_ERROR(err) {\
    fprintf(stderr, "hysplex error: "err"\n");\
    exit(1);\
}

#define HYSPLEX_INFO(info) {\
    fprintf(hysplex_stdout, "hysplex info: "info);\
}

#define HYSPLEX_WARN(warn) {\
    fprintf(hysple_stdout, "hysplex warn: "warn);\
}

#define HYSPLEX_PROGRESS(done, from) {\
    fprintf(hysplex_stdout, "hysplex info:\r                      \r%.2f%% completed.", ((float)done / (float)from) * 100);\
}

#define HYSPLEX_DO_EVAL_PAIR(iter_nr, warm_up, pre_run_stmt, post_run_stmt, func0, func1, func_args...) {\
        struct hysplex_stat hs[2] = { { #func0, 0, 0 }, { #func1, 0, 0 } };\
        if (warm_up) {\
            HYSPLEX_INFO("cache warming up... wait...\n");\
            for (size_t i = 0; i < iter_nr; i++) {\
                pre_run_stmt;\
                func_a(func_args);\
                post_run_stmt;\
                pre_run_stmt;\
                func_b(func_args);\
                post_run_stmt;\
                HYSPLEX_PROGRESS(i, iter_nr);\
            }\
        }\
        fprintf(hysplex_stdout, "\r             \rdone.\n");\
        HYSPLEX_INFO("'func0' vs. 'func1'... wait...\n");\
        for (size_t i = 0; i < iter_nr; i++) {\
            clock_t start, end;\
            pre_run_stmt;\
            start = clock();\
            func0(func_args);\
            end = clock();\
            post_run_stmt;\
            long double func0_exec_time = ((long double)(end - start)) / ((long double)CLOCK_PER_SECOND);\
            hs[0].exec_time_s += func0_exec_time;\
            pre_run_stmt;\
            start = clock();\
            func1(func_args);\
            end = clock();\
            post_run_stmt;\
            long double func1_exec_time = ((long double)(end - start)) / ((long double)CLOCK_PER_SECOND);\
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
        fprintf(hysplex_stdout, "\r             \rdone.\n");\
        if (hysplex_get_winner_function(hs, 2, iter_nr, 1) == -1) {\
            HYSPLEX_WARN("The evaluated performances are statistically equal. "\
                         "In fact you must not taking into consideration performance to pick one.")\
            exit(1);\
        }\
}

#define HYSPLEX_DO_EVAL_GROUP(iter_nr, warm_up, pre_run_stmt, post_run_stmt, group, group_nr, func_args...) {\
    ssize_t lg = 0;\
    for (ssize_t g = 1; g < group_nr; g++) {\
        struct hysplex_stat hs[2] = { { NULL, 0, 0 }, { NULL, 0, 0 } };\
        hs[0].func_name = group[lg].name;\
        hs[1].func_name = group[g].name;\
        if (warm_up) {\
            HYSPLEX_INFO("cache warming up... wait...\n");\
            for (size_t i = 0; i < iter_nr; i++) {\
                pre_run_stmt;\
                group[lg].func(func_args);\
                post_run_stmt;\
                pre_run_stmt;\
                group[g].func(func_args);\
                post_run_stmt;\
                HYSPLEX_PROGRESS(i, iter_nr);\
            }\
        }\
        fprintf(hysplex_stdout, "\r             \rdone.\n");\
        fprintf(hysplex_stdout, "hysplex info: '%s' vs. '%s'... wait...\n", group[lg].name, group[g].name);\
        for (size_t i = 0; i < iter_nr; i++) {\
            clock_t begin, end;\
            pre_run_stmt;\
            start = clock();\
            func0(func_args);\
            end = clock();\
            post_run_stmt;\
            long double func0_exec_time = ((long double)(end - start)) / ((long double)CLOCK_PER_SECOND);\
            hs[0].exec_time_s += func0_exec_time;\
            pre_run_stmt;\
            start = clock();\
            func1(func_args);\
            end = clock();\
            post_run_stmt;\
            long double func1_exec_time = ((long double)(end - start)) / ((long double)CLOCK_PER_SECOND);\
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
        fprintf(hysplex_stdout, "\r             \rdone.\n");\
        if ((lg = hysplex_get_winner_function(hs, 2, iter_nr, 1)) == -1) {\
            HYSPLEX_WARN("The evaluated performances are statistically equal. "\
                         "In fact you must not taking into consideration performance to pick one.")\
            exit(1);\
        }\
        if (lg == 1) {\
            lg = g;\
        }\
    }\
}

#define HYSPLEX_MAIN_BEGIN\
    int main(int argc, char **argv) {\
        hysplex_stdout = stdout;

#define HYSPLEX_MAIN_END\
    return 0;\
}

#define HYSPLEX_EVAL_PAIR(iter_nr, warm_up, pre_run_stmt, post_run_stmt, func0, func1, func_args...)\
    HYSPLEX_MAIN_BEGIN\
        HYSPLEX_BANNER\
        HYSPLEX_DO_EVAL_PAIR(iter_nr, warm_up, pre_run_stmt, post_run_stmt, func0, func1, func_args...)\
HYSPLEX_MAIN_END

#define HYSPLEX_EVAL_GROUP(iter_nr, warm_up, pre_run_stmt, post_run_stmt, group, func_args...)\
    HYSPLEX_MAIN_BEGIN\
        HYSPLEX_DO_EVAL_GROUP(iter_nr, warm_up, pre_run_stmt, post_run_stmt, group, HYSPLEX_FUNCTION_GROUP_SIZE(group),\
                              func_args...)\
HYSPLEX_MAIN_END

ssize_t hysplex_get_winner_function(struct hysplex_stat *hs, const size_t hs_nr, const size_t iter_nr, const int is_final);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HYSPLEX_H */
