//
// Created by 98max on 15/01/2024.
//

#ifndef MAXOS_KPRINT_H
#define MAXOS_KPRINT_H

#include <drivers/console/serial.h>

namespace MaxOS{

    void _kprintf(const char* format, ...);
    void _kitoa(char* buffer, int base, int number);
}

#endif // MAXOS_KPRINT_H
