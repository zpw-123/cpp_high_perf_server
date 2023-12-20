#ifndef UTIL_H
#define UTIL_H

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>
namespace cpp_high_perf {
    pid_t GetThreadId();
    uint32_t GetFiberId();
}

#endif