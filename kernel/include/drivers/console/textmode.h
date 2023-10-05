//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H

#include <common/types.h>
#include <drivers/driver.h>
#include <drivers/console/console.h>

namespace maxOS{

    namespace drivers {

        namespace console {

            class TextModeConsole : public Driver, public Console
            {

                public:
                    TextModeConsole();
                    ~TextModeConsole();

                    common::uint16_t getWidth();
                    common::uint16_t getHeight();

                    void putChar(common::uint16_t x, common::uint16_t y, char c);
                    void setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor foreground);
                    void setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor background);

                    char getChar(common::uint16_t x, common::uint16_t y);
                    ConsoleColor getForegroundColor(common::uint16_t x, common::uint16_t y);
                    ConsoleColor getBackgroundColor(common::uint16_t x, common::uint16_t y);

                protected:
                    common::uint16_t* videoMemory;
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
