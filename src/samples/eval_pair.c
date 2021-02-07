#include <hysplex.h>
#include <stdio.h>
#include <unistd.h>

static int plain_sum(const int a, const int b) {
    return a + b;
}

static int delayed_sum(const int a, const int b) {
    usleep(1);
    return a + b;
}

HYSPLEX_EVAL_PAIR(100000, 1, {}, {}, plain_sum, delayed_sum, 1, 2);
