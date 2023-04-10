//
// Created by 98max on 10/04/2023.
//

#ifndef MAXOS_COMMON_TIME_H
#define MAXOS_COMMON_TIME_H

#include <common/types.h>

namespace maxOS{

    namespace common{

        struct Time{

            uint16_t year;
            uint8_t month;
            uint8_t day;

            uint8_t hour;
            uint8_t minute;
            uint8_t second;

        };

    }

}

#endif //MAXOS_COMMON_TIME_H
