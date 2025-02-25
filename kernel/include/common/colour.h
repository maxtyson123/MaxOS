//
// Created by 98max on 30/03/2023.
//

#ifndef MAXOS_COMMON_COLOUR_H
#define MAXOS_COMMON_COLOUR_H

#include <common/string.h>
#include <stdint.h>

namespace MaxOS{

    namespace common
    {

      const char* const ANSI_COLOURS[] = {
        "\033[0;30m", // FG Black
        "\033[0;31m", // FG Red
        "\033[0;32m", // FG Green
        "\033[0;33m", // FG Yellow
        "\033[0;34m", // FG Blue
        "\033[0;35m", // FG Magenta
        "\033[0;36m", // FG Cyan
        "\033[0;37m", // FG White
        "\033[0;40m", // BG Black
        "\033[0;41m", // BG Red
        "\033[0;42m", // BG Green
        "\033[0;43m", // BG Yellow
        "\033[0;44m", // BG Blue
        "\033[0;45m", // BG Magenta
        "\033[0;46m", // BG Cyan
        "\033[0;47m",  // BG White
        "\033[0m"     // Reset
      };

      enum ANSIColour
      {
        FG_Black   = 0,
        FG_Red     = 1,
        FG_Green   = 2,
        FG_Yellow  = 3,
        FG_Blue    = 4,
        FG_Magenta = 5,
        FG_Cyan    = 6,
        FG_White   = 7,
        BG_Black   = 8,
        BG_Red     = 9,
        BG_Green   = 10,
        BG_Yellow  = 11,
        BG_Blue    = 12,
        BG_Magenta = 13,
        BG_Cyan    = 14,
        BG_White   = 15,
        Reset      = 16
      };

      enum ConsoleColour
      {
        Unititialised = 0xFF,
        Black        = 0x00,
        Blue         = 0x01,
        Green        = 0x02,
        Cyan         = 0x03,
        Red          = 0x04,
        Magenta      = 0x05,
        Brown        = 0x06,
        LightGrey    = 0x07,
        DarkGrey     = 0x08,
        LightBlue    = 0x09,
        LightGreen   = 0x0A,
        LightCyan    = 0x0B,
        LightRed     = 0x0C,
        LightMagenta = 0x0D,
        Yellow       = 0x0E,
        White        = 0x0F
      };

        /**
         * @class Colour
         * @brief Stores the red, green, blue and alpha values of a colour
         */
        class Colour
        {
            private:
                void parse_hex_string(string hex_string);
                void parse_ansi_string(string ansi_string);

                void parse_console_colour(ConsoleColour colour);

            public:
                uint8_t red { 0 };
                uint8_t green { 0 };
                uint8_t blue { 0 };
                uint8_t alpha { 0 };


                Colour();
                Colour(ConsoleColour colour);
                Colour(string string);
                Colour(uint8_t red, uint8_t green, uint8_t blue);
                Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

                ~Colour();

                ConsoleColour to_console_colour() const;
        };

    }

}

#endif //MAXOS_COMMON_COLOUR_H
