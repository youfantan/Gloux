#pragma once

#include "gloux.h"
#include <string>
#include <iostream>

#define ERRIF(cond, msg) errif(cond, msg, __FILE_NAME__, __LINE__)

void errif(bool condition, const std::string& message, const char* filename, int line) {
#ifdef ENABLE_DEBUG_LOG
    if (condition) {
        int ERRNO = errno;
        printf("%s (%d) at %s : %d\n", message.c_str(), ERRNO, filename, line);
    }
#endif
}