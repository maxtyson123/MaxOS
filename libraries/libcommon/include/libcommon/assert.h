//
// Created by 98max on 12/4/2025.
//

#ifndef MAXOS_COMMON_ASSERT_H
#define MAXOS_COMMON_ASSERT_H


#ifdef MAXOS_KERNEL

#include <common/logger.h>
#define ASSERT(condition, format, ...) Logger::assert(condition, format, ##__VA_ARGS__)


#else
#include "libkpi/syscalls.h"
#include "libkpi/ipc/messages.h"
#include <libkpi/syscalls.h>
#include <libkpi/processes/process.h>
#define ASSERT(condition, format, ...) user_assert_handler(condition, format, ##__VA_ARGS__)
inline void user_assert_handler(bool condition, const char* format, ...) {
	if (condition)
		return;

    // Create a pointer to the data
    va_list parameters;
    va_start(parameters, format);

    MaxOS::KPI::klog("%h\n");
	MaxOS::KPI::klog("ASSERT FAILED: %s\n", MaxOS::String::formatted(format, parameters).c_str());
	MaxOS::KPI::processes::exit(1);
}


#endif

#endif //MAXOS_COMMON_ASSERT_H
