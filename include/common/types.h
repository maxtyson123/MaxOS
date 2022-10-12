#ifndef MAX_OS_COMMON_TYPES_H
#define MAX_OS_COMMON_TYPES_H

namespace maxos
{
    namespace common
    {
        typedef char                     int8_t;
        typedef unsigned char           uint8_t;
        typedef short                   int16_t;
        typedef unsigned short         uint16_t;
        typedef int                     int32_t;
        typedef unsigned int           uint32_t;
        typedef long long int           int64_t;
        typedef unsigned long long int uint64_t;
        typedef const char*              string;
    }
}



#endif
//This ensures all the byte defs are the same, usefull for other compliers