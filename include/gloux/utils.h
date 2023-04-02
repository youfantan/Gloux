#pragma once

#include <cstdio>
#include <cstdlib>

#define ERRIF(expr, msg) if (!expr) error(msg, __FILE_NAME__, __LINE__)

static void error(const char* msg, const char *file_name, int line) {
    perror(msg);
    printf("\tat %s : %d\n", file_name, line);
    exit(EXIT_FAILURE);
}