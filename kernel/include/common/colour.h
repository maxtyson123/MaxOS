//
// Created by 98max on 30/03/2023.
//

#ifndef MAXOS_COMMON_COLOUR_H
#define MAXOS_COMMON_COLOUR_H

#include <stdint.h>

namespace maxOS{

    namespace common
    {

        /**
         * @class Colour
         * @brief Stores the red, green, blue and alpha values of a colour
         */
        class Colour
        {
            public:
                uint8_t red { 0 };
                uint8_t green { 0 };
                uint8_t blue { 0 };
                uint8_t alpha { 0 };


                Colour();
                Colour(uint8_t red, uint8_t green, uint8_t blue);
                Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
                ~Colour();
        };

    }

}

#endif //MAXOS_COMMON_COLOUR_H
