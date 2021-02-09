/*
 * Copyright (c) 2021, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <hysplex.h>
#include <sys/stat.h>
#include <fcntl.h>

int stat_based_file_exists(const char *filepath) {
    if (filepath == NULL) {
        return 0;
    }
    struct stat st;
    return (stat(filepath, &st) == 0);
}

int fopen_based_file_exists(const char *filepath) {
    if (filepath == NULL) {
        return 0;
    }
    FILE *fp;
    int retval = ((fp = fopen(filepath, "r")) != NULL);
    if (retval) {
        fclose(fp);
    }
    return retval;
}

int open_based_file_exists(const char *filepath) {
    if (filepath == NULL) {
        return 0;
    }
    int fd;
    int retval = ((fd = open(filepath, O_RDONLY)) != -1);
    if (retval > -1) {
        close(fd);
    }
    return retval;
}

HYSPLEX_FUNCTION_GROUP_BEGIN(func_group)
    HYSPLEX_REGISTER_FUNCTION(fopen_based_file_exists),
    HYSPLEX_REGISTER_FUNCTION(stat_based_file_exists),
    HYSPLEX_REGISTER_FUNCTION(open_based_file_exists),
HYSPLEX_FUNCTION_GROUP_END

HYSPLEX_EVAL_GROUP(100000, 1, {}, {}, func_group, "eval_group.c");
