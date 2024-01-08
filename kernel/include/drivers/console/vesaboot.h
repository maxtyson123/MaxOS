//
// Created by 98max on 10/2/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H

#include <stdint.h>
#include <gui/font.h>
#include <drivers/driver.h>
#include <drivers/console/console.h>
#include <common/logo.h>

namespace maxOS{

    namespace drivers {

        namespace console {

            /**
             * @class VESABootConsole
             * @brief Driver for the VESA Console during boot, handles the printing of characters and strings to the screen using VESA
             */
            class VESABootConsole : public Driver, public Console
            {

                protected:
                    uint16_t* m_video_memory;
                    common::GraphicsContext* m_graphics_context;
                    gui::Font m_font;

                public:
                    VESABootConsole(common::GraphicsContext*);
                    ~VESABootConsole();

                    uint16_t width() final;
                    uint16_t height() final;

                    void put_character(uint16_t x, uint16_t y, char) final;
                    void set_foreground_color(uint16_t x, uint16_t y, ConsoleColour) final;
                    void set_background_color(uint16_t x, uint16_t y, ConsoleColour) final;

                    char get_character(uint16_t x, uint16_t y) final;
                    ConsoleColour get_foreground_color(uint16_t x, uint16_t y) final;
                    ConsoleColour get_background_color(uint16_t x, uint16_t y) final;

                    common::Colour console_colour_to_vesa(ConsoleColour);

                    void print_logo();
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
