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

                uint8_t x = 0, y = 0;    //Cursor Location

                void put_string_gui(char* str, gui::Text lines[15]);
                void put_string(char* str, bool clearLine = false);
                void put_hex(uint8_t key);

                void moveCursor(uint8_t C_x, uint8_t C_y);
        };


    };

}

class printf {

};


#endif //MAXOS_PRINTF_H