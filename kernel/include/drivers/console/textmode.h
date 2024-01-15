//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H

#include <stdint.h>
#include <drivers/driver.h>
#include <drivers/console/console.h>

namespace MaxOS{

    namespace drivers {

        namespace console {

            /**
             * @class TextModeConsole
             * @brief Driver for the text mode console, handles the printing of characters and strings to the screen using VGA
             */
            class TextModeConsole : public Driver, public Console
            {
                protected:
                    uint16_t* m_video_memory { (uint16_t*)0xB8000 };

                public:
                    TextModeConsole();
                    ~TextModeConsole();

                    uint16_t width() final;
                    uint16_t height() final;

                    void put_character(uint16_t x, uint16_t y, char c) final;
                    void set_foreground_color(uint16_t x, uint16_t y, ConsoleColour) final;
                    void set_background_color(uint16_t x, uint16_t y, ConsoleColour) final;

                    char get_character(uint16_t x, uint16_t y) final;
                    ConsoleColour get_foreground_color(uint16_t x, uint16_t y) final;
                    ConsoleColour get_background_color(uint16_t x, uint16_t y) final;
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
