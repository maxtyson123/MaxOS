#ifndef MAX_OS_COMMON_TYPES_H
#define MAX_OS_COMMON_TYPES_H

namespace maxOS
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
        typedef uint32_t                 size_t;    //On 32bit system the mem adresses are 32bit
    }
}



#endif
//This ensures all the byte defs are the same, usefull for other compliers