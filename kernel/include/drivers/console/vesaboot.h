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
#include <common/logo_kp.h>
#include <common/colour.h>
#include <system/cpu.h>
#include <memory/memoryIO.h>

namespace MaxOS{

    namespace drivers {

        namespace console {

            /**
             * @class VESABootConsole
             * @brief Driver for the VESA Console during boot, handles the printing of characters and strings to the screen using VESA
             */
            class VESABootConsole : public Driver, public Console
            {

                private:
                    uint16_t* m_video_memory;
                    common::GraphicsContext* m_graphics_context;
                    gui::Font m_font;

                    uint8_t ansi_code_length = -1;
                    char ansi_code[8] = "0000000";
                    common::ConsoleColour m_foreground_color = common::ConsoleColour::Unititialised;
                    common::ConsoleColour m_background_color = common::ConsoleColour::Unititialised;

                public:
                    VESABootConsole(common::GraphicsContext*);
                    ~VESABootConsole();

                    uint16_t width() final;
                    uint16_t height() final;

                    void put_character(uint16_t x, uint16_t y, char) final;
                    void set_foreground_color(uint16_t x, uint16_t y, common::ConsoleColour) final;
                    void set_background_color(uint16_t x, uint16_t y, common::ConsoleColour) final;

                    void scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground, common::ConsoleColour background, char fill) final;

                    char get_character(uint16_t x, uint16_t y) final;
                    common::ConsoleColour get_foreground_color(uint16_t x, uint16_t y) final;
                    common::ConsoleColour get_background_color(uint16_t x, uint16_t y) final;

                    void print_logo();
                    void print_logo_kernel_panic();
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_VESABOOTCONSOLE_H
