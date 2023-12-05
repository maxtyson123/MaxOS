//
// Created by 98max on 10/2/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H

#include <stdint.h>
#include <gui/font.h>
#include <drivers/driver.h>
#include <drivers/console/console.h>

namespace maxOS{

    namespace drivers {

        namespace console {

            class VESABootConsole : public Driver, public Console
            {

            public:
                VESABootConsole(common::GraphicsContext* graphicsContext);
                ~VESABootConsole();

                uint16_t getWidth();
                uint16_t getHeight();

                void putChar(uint16_t x, uint16_t y, char c);
                void setForegroundColor(uint16_t x, uint16_t y, ConsoleColour foreground);
                void setBackgroundColor(uint16_t x, uint16_t y, ConsoleColour background);

                char getChar(uint16_t x, uint16_t y);
                ConsoleColour getForegroundColor(uint16_t x, uint16_t y);
                ConsoleColour getBackgroundColor(uint16_t x, uint16_t y);

                common::Colour consoleColourToVESA(ConsoleColour colour);

                uint16_t videoMemory[10881];     //128*85

            protected:

                common::GraphicsContext* graphicsContext;
                gui::AmigaFont font;
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
