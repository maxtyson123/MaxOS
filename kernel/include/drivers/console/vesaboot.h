//
// Created by 98max on 10/2/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H

#include <common/types.h>
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

                common::uint16_t getWidth();
                common::uint16_t getHeight();

                void putChar(common::uint16_t x, common::uint16_t y, char c);
                void setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor foreground);
                void setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor background);

                char getChar(common::uint16_t x, common::uint16_t y);
                ConsoleColor getForegroundColor(common::uint16_t x, common::uint16_t y);
                ConsoleColor getBackgroundColor(common::uint16_t x, common::uint16_t y);

                common::Colour consoleColourToVESA(ConsoleColor colour);

                common::uint16_t videoMemory[10881];     //128*85

            protected:

                common::GraphicsContext* graphicsContext;
                gui::AmigaFont font;
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
