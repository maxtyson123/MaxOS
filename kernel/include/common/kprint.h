//
// Created by 98max on 15/01/2024.
//

#ifndef MAXOS_KPRINT_H
#define MAXOS_KPRINT_H

#include <drivers/console/serial.h>
#include <stdarg.h>


#define _kprintf(format, ...) \
    _kprintf_internal(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)


void _kprintf_internal(const char* file, int line, const char* func, const char* format, ...);

#endif // MAXOS_KPRINT_H
