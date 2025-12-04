//
// Created by 98max on 12/4/2025.
//

#ifndef MAXOS_COMMON_ASSERT_H
#define MAXOS_COMMON_ASSERT_H


#ifdef MAXOS_KERNEL

#include <common/logger.h>
#define ASSERT(condition, format, ...) Logger::assert(condition, format, ##__VA_ARGS__)


#else
#include <syscalls.h>
#include <processes/process.h>
#define ASSERT(condition, format, ...) user_assert_handler(condition, format, ##__VA_ARGS__)
inline void user_assert_handler(bool condition, const char* format, ...) {
	if (condition)
		return;

	MaxOS::KPI::klog("Assert failed");
	MaxOS::KPI::processes::exit(1);
}


#endif

#endif //MAXOS_COMMON_ASSERT_H
