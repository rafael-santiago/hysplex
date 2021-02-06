/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <hysplex.h>
#include <string.h>
#include <ctype.h>

static long double HYSPLEX_CHI_SQUARE_ALPHA[] = {
    /*90%    95%    97%    99%    100%*/
    2.706, 3.841, 5.024, 6.635, 10.828,
};

static int hysplex_get_option_index(size_t *data_offset, const char *option, const int bool);

static inline long double hysplex_get_chi_square_alpha(void);

ssize_t hysplex_get_winner_function(char *buf, const size_t buf_size, struct hysplex_stat *hs, const size_t hs_nr,
                                    const size_t iter_nr, const int is_final) {

    char *bp = buf;
    size_t bp_size = buf_size;

    if (hs == NULL || hs_nr == 0 || iter_nr == 0) {
        HYSPLEX_SNPRINTF(bp, bp_size, "hysplex error: invalid arguments.");
        exit(1);
    }

    struct hysplex_stat *s[2] = { NULL, NULL };
    size_t i = 0;
    for (size_t h = 0; h < hs_nr; h++) {
        if (hs[h].score > 0) {
            if (i == 2) {
                HYSPLEX_ERROR("hysplex_stat has more than two winner candidates.");
                exit(1);
            }
            s[i++] = &hs[h];
        }
    }

    if (s[0] == NULL && s[1] == NULL) {
        HYSPLEX_SNPRINTF(bp, bp_size, "hysplex error: no winners.\n");
        return -1;
    } else if (s[0] != NULL && s[1] == NULL) {
        HYSPLEX_SNPRINTF(bp, bp_size, "== The winner function is '%s'. It won %d in %d. Unable to get more stats.\n",
                                s[0]->func_name, s[0]->score, iter_nr);
        return 0;
    } else if (s[1] != NULL && s[0] == NULL) {
        HYSPLEX_SNPRINTF(bp, bp_size, "== The winner function is '%s'. It won %d in %d. Unable to get more stats.\n",
                                s[1]->func_name, s[1]->score, iter_nr);
        return 1;
    }

    size_t ties_nr = (s[0]->score + s[1]->score - iter_nr) / 2;
    HYSPLEX_SNPRINTF(bp, bp_size, "== Hysplex %s stats\n\n", ((is_final) ? "final" : "intermediate"));
    HYSPLEX_SNPRINTF(bp, bp_size, "== Functions '%s' and '%s' were executed %d time(s).\n", s[0]->func_name,
                                                                                     s[1]->func_name, iter_nr);
    HYSPLEX_SNPRINTF(bp, bp_size, "== Function '%s' has won %d time(s).\n", s[0]->func_name, s[0]->score);
    HYSPLEX_SNPRINTF(bp, bp_size, "== Function '%s' has won %d time(s).\n", s[1]->func_name, s[1]->score);
    HYSPLEX_SNPRINTF(bp, bp_size, "== Total of tied executions: %d.\n", ties_nr);
    HYSPLEX_SNPRINTF(bp, bp_size, "== Total of iterations with a winner: %d\n", iter_nr - ties_nr);
    HYSPLEX_SNPRINTF(bp, bp_size, "== The average execution time of function '%s' was about %Lf secs.\n", s[0]->func_name,
                                                                                                  s[0]->exec_time_s /
                                                                                               (long double)iter_nr);
    HYSPLEX_SNPRINTF(bp, bp_size, "== The average execution time of function '%s' was about %Lf secs.\n", s[1]->func_name,
                                                                                                 s[1]->exec_time_s /
                                                                                               (long double)iter_nr);
    int wi = ((s[0]->score > s[1]->score) ? 0 : (s[1]->score > s[0]->score) ? 1 : -1);

    if (wi > -1) {
        HYSPLEX_SNPRINTF(bp, bp_size, "\n== The winner function is '%s'.\n\n", s[wi]->func_name);
        long double freqs[2] = { s[0]->score - ties_nr, s[1]->score - ties_nr };
        size_t effective_iters = iter_nr - ties_nr;
        long double exp_freq = effective_iters / 2;
        freqs[0] = (((long double)freqs[0] - exp_freq) * ((long double)freqs[0] - exp_freq)) / exp_freq;
        freqs[1] = (((long double)freqs[1] - exp_freq) * ((long double)freqs[1] - exp_freq)) / exp_freq;
        long double chi_square = freqs[0] + freqs[1];

        int is_relevant = (chi_square > hysplex_get_chi_square_alpha());

        HYSPLEX_SNPRINTF(bp, bp_size, "== Chi-square = %.3Lf (certainty = %s%), ", chi_square, hysplex_get_option("certainty-perc", "95"));
        HYSPLEX_SNPRINTF(bp, bp_size, "'%s' %s statistically faster than '%s'.\n", s[wi]->func_name,
                                      (is_relevant ? "is" : "is NOT"), s[!wi]->func_name);

        if (!is_relevant) {
            wi = -1;
        }
    } else {
        HYSPLEX_SNPRINTF(bp, bp_size, "== No winner. We have a tie.\n\n");
    }

    return wi;
}

void hysplex_set_argc_argv(const int argc, char **argv) {
    hysplex_argc = argc;
    hysplex_argv = argv;
}

static int hysplex_get_option_index(size_t *data_offset, const char *option, const int bool) {
    if (option == NULL || (!bool && data_offset == NULL)) {
        return -1;
    }

    char temp[4096];
    snprintf(temp, sizeof(temp) - 1, ((bool) ? "--%s" : "--%s="), option);
    if (!bool) {
        *data_offset = 0;
    }

    for (size_t a = 0; a < hysplex_argc; ++a) {
        int has = ((bool) ? (strcmp(temp, hysplex_argv[a]) == 0) : (strstr(hysplex_argv[a], temp) == &hysplex_argv[a][0]));
        if (has){
            if (!bool) {
                *data_offset = strlen(temp);
            }
            return a;
        }
    }

    return -1;
}

const char *hysplex_get_option(const char *option, const char *default_option) {
    size_t data_offset;
    int o = hysplex_get_option_index(&data_offset, option, 0);
    return ((o > -1) ? ((&hysplex_argv[o][0]) + data_offset) : default_option);
}

int hysplex_get_bool_option(const char *option, const int default_option) {
    return ((hysplex_get_option_index(NULL, option, 1) > -1) ? 1 : default_option);
}

int hysplex_is_valid_number(const char *number, const size_t number_size) {
    if (number == NULL || number_size == 0) {
        return 0;
    }
    const char *np = number, *np_end = np + number_size;
    while (np != np_end) {
        if (!isdigit(*np)) {
            return 0;
        }
        np++;
    }
    return 1;
}

static inline long double hysplex_get_chi_square_alpha(void) {
    static ssize_t alpha_index = -1;
    if (alpha_index == -1) {
        const char *certainty_perc = hysplex_get_option("certainty-perc", "95");
        if (certainty_perc == NULL) {
            HYSPLEX_ERROR("Invalid data in certainty-perc option.");
            exit(1);
        }
        if (strcmp(certainty_perc, "90") == 0) {
            alpha_index = 0;
        } else if (strcmp(certainty_perc, "95") == 0) {
            alpha_index = 1;
        } else if (strcmp(certainty_perc, "97") == 0) {
            alpha_index = 2;
        } else if (strcmp(certainty_perc, "99") == 0) {
            alpha_index = 3;
        } else if (strcmp(certainty_perc, "100") == 0) {
            alpha_index = 4;
        } else {
            HYSPLEX_ERROR("Unknown data in certainty-perc option.");
            exit(1);
        }
    }
    return HYSPLEX_CHI_SQUARE_ALPHA[alpha_index];
}

int hysplex_validate_user_options(void) {
    const char *option = hysplex_get_option("certainty-perc", "95");
    if (strcmp(option, "90") != 0 &&
        strcmp(option, "95") != 0 &&
        strcmp(option, "97") != 0 &&
        strcmp(option, "99") != 0 &&
        strcmp(option, "100") != 0) {
        HYSPLEX_ERROR("Invalid data in 'certainty-perc' option. It must be '90', '95' (default), '97', '99' or '100'.");
        return 0;
    }
    option = hysplex_get_option("iterations", "1");
    if (!hysplex_is_valid_number(option, strlen(option)) || strtoul(option, NULL, 10) == 0) {
        HYSPLEX_ERROR("Invalid data in 'iterations' option. It must be a valid positive integer greater than zero.");
        return 0;
    }
    return 1;
}
