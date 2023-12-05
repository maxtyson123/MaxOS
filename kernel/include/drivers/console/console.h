//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_H

#include <stdint.h>
#include <common/outputStream.h>

namespace maxOS {

    namespace drivers {

        namespace console {

            enum ConsoleColour
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

                virtual uint16_t getWidth();
                virtual uint16_t getHeight();

                virtual void putChar(uint16_t x, uint16_t y, char c);
                virtual char getChar(uint16_t x, uint16_t y);

                virtual void setForegroundColor(uint16_t x, uint16_t y, ConsoleColour foreground);
                virtual void setBackgroundColor(uint16_t x, uint16_t y, ConsoleColour background);

                virtual ConsoleColour getForegroundColor(uint16_t x, uint16_t y);
                virtual ConsoleColour getBackgroundColor(uint16_t x, uint16_t y);


                virtual void putChar(uint16_t x, uint16_t y, char c, ConsoleColour foreground, ConsoleColour background);
                virtual void putString(uint16_t x, uint16_t y, string s, ConsoleColour foreground = LightGrey, ConsoleColour background = Black);
                virtual void scrollUp();
                virtual void scrollUp(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground = LightGrey, ConsoleColour background = Black, char fill=' ');
                virtual void clear();
                virtual void clear(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground = LightGrey, ConsoleColour background = Black, char fill=' ');
                virtual void invertColors(uint16_t x, uint16_t y);
            };

            class ConsoleArea : public Console
            {
            protected:
                Console* console;
                uint16_t left;
                uint16_t top;
                uint16_t width;
                uint16_t height;
            public:
                ConsoleArea(Console* console, uint16_t left, uint16_t top, uint16_t width, uint16_t height);
                ConsoleArea(Console* console, uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground, ConsoleColour background);
                ~ConsoleArea();

                uint16_t getWidth();
                uint16_t getHeight();

                void putChar(uint16_t x, uint16_t y, char c);
                void setForegroundColor(uint16_t x, uint16_t y, ConsoleColour foreground);
                void setBackgroundColor(uint16_t x, uint16_t y, ConsoleColour background);

                char getChar(uint16_t x, uint16_t y);
                ConsoleColour getForegroundColor(uint16_t x, uint16_t y);
                ConsoleColour getBackgroundColor(uint16_t x, uint16_t y);
            };

            class ConsoleStream : public common::OutputStream
            {
            protected:
                Console* console;

                ConsoleColour foreground;
                ConsoleColour background;

            public:
                uint16_t cursorX;
                uint16_t cursorY;

                ConsoleStream(Console* console);
                ~ConsoleStream();
                void writeChar(char c);
                void setCursor(uint16_t x, uint16_t y);
            };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_H
