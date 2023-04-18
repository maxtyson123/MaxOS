//
// Created by 98max on 21/10/2022.
//

#ifndef MAXOS_PRINTF_H
#define MAXOS_PRINTF_H

#include <common/types.h>
#include <gui/widgets/text.h>

namespace maxOS{

    namespace common{

        class Console{
            public:
                Console();
                ~Console();

                uint8_t x = 0, y = 0;            //Cursor Location
                uint8_t lim_x = 0, lim_y = 0;    //Cursor Limits
                uint8_t ini_x = 0, ini_y = 0;    //Cursor Start

                void put_string(char* str, bool clearLine = false);
                void put_hex(uint8_t key);
                char* int_to_string(int i);

                void moveCursor(uint8_t C_x, uint8_t C_y);
                void backspace();
        };


        int strcmp(const char* s1, const char* s2);

    };

}




#endif //MAXOS_PRINTF_H
