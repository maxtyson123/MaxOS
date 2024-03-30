//
// Created by 98max on 15/01/2024.
//

#ifndef MAXOS_KPRINT_H
#define MAXOS_KPRINT_H

#include <drivers/console/serial.h>
#include <stdarg.h>


#define _kprintf(format, ...) \
    _kprintf_internal(0, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

#define ASSERT(condition, format, ...) \
    if(!(condition)) { \
        _kprintf_internal(3, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__); \
        while(1); \
    }

void _kprintf_internal(uint8_t type, const char* file, int line, const char* func, const char* format, ...);

#endif // MAXOS_KPRINT_H
