/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <hysplex.h>

#define HYSPLEX_CHI_SQUARE_ALPHA 3.841

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

    size_t ties_nr = s[0]->score + s[1]->score - iter_nr;
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

        int is_relevant = (chi_square > HYSPLEX_CHI_SQUARE_ALPHA);

        HYSPLEX_SNPRINTF(bp, bp_size, "== Chi-square = %.3Lf (sig = 0.05), ", chi_square);
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

#undef HYSPLEX_CHI_SQUARE_ALPHA
