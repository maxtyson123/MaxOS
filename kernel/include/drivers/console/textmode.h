//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H

#include <stdint.h>
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

                    uint16_t getWidth();
                    uint16_t getHeight();

                    void putChar(uint16_t x, uint16_t y, char c);
                    void setForegroundColor(uint16_t x, uint16_t y, ConsoleColour foreground);
                    void setBackgroundColor(uint16_t x, uint16_t y, ConsoleColour background);

                    char getChar(uint16_t x, uint16_t y);
                    ConsoleColour getForegroundColor(uint16_t x, uint16_t y);
                    ConsoleColour getBackgroundColor(uint16_t x, uint16_t y);

                protected:
                    uint16_t* videoMemory;
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
