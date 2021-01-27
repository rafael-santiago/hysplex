/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <hysplex.h>

#define HYSPLEX_CHI_SQUARE_ALPHA 3.841

ssize_t hysplex_get_winner_function(struct hysplex_stat *hs, const size_t hs_nr, const size_t iter_nr, const int is_final) {
    if (hs == NULL || hs_nr == 0 || iter_nr == 0) {
        HYSPLEX_ERROR("invalid arguments.");
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

    size_t ties_nr = s[0]->score + s[1]->score - iter_nr;

    fprintf(hysplex_stdout, "== Hysplex %s stats\n\n", (is_final) ? "final" : "intermediate");
    fprintf(hysplex_stdout, "== Functions '%s' and '%s' was executed %d time(s).\n", s[0]->func_name,
                                                                                     s[1]->func_name, iter_nr);
    fprintf(hysplex_stdout, "== Function '%s' has won %d time(s).\n", s[0]->func_name, s[0]->score);
    fprintf(hysplex_stdout, "== Function '%s' has won %d time(s).\n", s[1]->func_name, s[1]->score);
    fprintf(hysplex_stdout, "== Total of tied executions: %d.\n", ties_nr);
    fprintf(hysplex_stdout, "== The average execution time of function '%s' was about %Lf secs.\n", s[0]->func_name,
                                                                                                  s[0]->exec_time_s /
                                                                                               (long double)iter_nr);
    fprintf(hysplex_stdout, "== The average execution time of function '%s' was about %Lf secs.\n", s[1]->func_name,
                                                                                                 s[1]->exec_time_s /
                                                                                               (long double)iter_nr);

    int wi = ((s[0]->score > s[1]->score) ? 0 : (s[1]->score > s[0]->score) ? 1 : -1);

    if (wi > -1) {
        fprintf(stdout, "== The winner function is '%s'.\n\n", s[wi]->func_name);
    } else {
        fprintf(stdout, "== No winner. We have a tie.\n\n");
    }

    long double freqs[2] = { s[0]->score - ties_nr, s[1]->score - ties_nr };
    size_t effective_iters = iter_nr - ties_nr;
    long double exp_freq = effective_iters / 2;
    freqs[0] = (((long double)freqs[0] - exp_freq) * ((long double)freqs[0] - exp_freq)) / exp_freq;
    freqs[1] = (((long double)freqs[1] - exp_freq) * ((long double)freqs[1] - exp_freq)) / exp_freq;
    long double chi_square = freqs[0] + freqs[1];

    int is_relevant = (chi_square > HYSPLEX_CHI_SQUARE_ALPHA);

    fprintf(hysplex_stdout, "== Chi-square = %Lf (sig = 0.05), ", chi_square);
    fprintf(hysplex_stdout, "'%s' %s statistically faster than '%s'.\n", s[wi]->func_name,
                                  (is_relevant ? "is" : "is NOT"), s[!wi]->func_name);

    if (!is_relevant) {
        wi = -1;
    }

    return wi;
}

#undef HYSPLEX_CHI_SQUARE_ALPHA