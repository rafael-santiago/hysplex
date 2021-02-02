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

static int sum_with_copy(const int a, const int b) {
    int aa = a;
    int bb = b;
    return aa + bb;
}

static int sum_with_xor_swap(const int a, const int b) {
    int aa = a ^ b;
    int bb = aa ^ b;
    aa ^= bb;
    return aa + bb;
}

HYSPLEX_EVAL_PAIR(100000, 1, {}, {}, sum_with_copy, sum_with_xor_swap, 1, 2);
