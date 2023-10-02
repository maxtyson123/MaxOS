//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_H

#include <common/types.h>
#include <common/outputStream.h>

namespace maxOS {

    namespace drivers {

        namespace console {

            enum ConsoleColor
            {
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

            class Console
            {
            public:
                Console();
                ~Console();

                virtual common::uint16_t getWidth();
                virtual common::uint16_t getHeight();

                virtual void putChar(common::uint16_t x, common::uint16_t y, char c);
                virtual void setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor foreground);
                virtual void setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor background);

                virtual char getChar(common::uint16_t x, common::uint16_t y);
                virtual ConsoleColor getForegroundColor(common::uint16_t x, common::uint16_t y);
                virtual ConsoleColor getBackgroundColor(common::uint16_t x, common::uint16_t y);


                virtual void putChar(common::uint16_t x, common::uint16_t y, char c, ConsoleColor foreground, ConsoleColor background);
                virtual void putString(common::uint16_t x, common::uint16_t y, common::string s, ConsoleColor foreground = LightGrey, ConsoleColor background = Black);
                virtual void scrollUp();
                virtual void scrollUp(common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColor foreground = LightGrey, ConsoleColor background = Black, char fill=' ');
                virtual void clear();
                virtual void clear(common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColor foreground = LightGrey, ConsoleColor background = Black, char fill=' ');
                virtual void invertColors(common::uint16_t x, common::uint16_t y);
            };

            class ConsoleArea : public Console
            {
            protected:
                Console* console;
                common::uint16_t left;
                common::uint16_t top;
                common::uint16_t width;
                common::uint16_t height;
            public:
                ConsoleArea(Console* console, common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height);
                ConsoleArea(Console* console, common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColor foreground, ConsoleColor background);
                ~ConsoleArea();

                common::uint16_t getWidth();
                common::uint16_t getHeight();

                void putChar(common::uint16_t x, common::uint16_t y, char c);
                void setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor foreground);
                void setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColor background);

                char getChar(common::uint16_t x, common::uint16_t y);
                ConsoleColor getForegroundColor(common::uint16_t x, common::uint16_t y);
                ConsoleColor getBackgroundColor(common::uint16_t x, common::uint16_t y);
            };

            class ConsoleStream : public common::OutputStream
            {
            protected:
                Console* console;
                common::uint16_t cursorX;
                common::uint16_t cursorY;

            public:
                ConsoleStream(Console* console);
                ~ConsoleStream();
                void writeChar(char c);
                void setCursor(common::uint16_t x, common::uint16_t y);
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_H
