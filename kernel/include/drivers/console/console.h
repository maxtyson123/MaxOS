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

            /**
             * @class Console
             * @brief Abstract class for a console, allows for the printing of characters and strings with colours
             */
            class Console
            {
              public:
                  Console();
                  ~Console();

                  virtual uint16_t width();
                  virtual uint16_t height();

                  virtual void put_character(uint16_t x, uint16_t y, char c);
                  virtual char get_character(uint16_t x, uint16_t y);

                  virtual void set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground);
                  virtual void set_background_color(uint16_t x, uint16_t y, ConsoleColour background);

                  virtual ConsoleColour get_foreground_color(uint16_t x, uint16_t y);
                  virtual ConsoleColour get_background_color(uint16_t x, uint16_t y);


                  virtual void put_character(uint16_t x, uint16_t y, char c, ConsoleColour foreground, ConsoleColour background);
                  virtual void put_string(uint16_t x, uint16_t y, string s, ConsoleColour foreground = LightGrey, ConsoleColour background = Black);

                  virtual void scroll_up();
                  virtual void scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground = LightGrey, ConsoleColour background = Black, char fill= ' ');

                  virtual void clear();
                  virtual void clear(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground = LightGrey, ConsoleColour background = Black, char fill=' ');

                  virtual void invert_colors(uint16_t x, uint16_t y);

            };

            /**
             * @class ConsoleArea
             * @brief A console that is a subsection of another console, limited by a width and height
             */
            class ConsoleArea : public Console
            {
              protected:
                  Console* m_console;
                  uint16_t m_left;
                  uint16_t m_top;
                  uint16_t m_width;
                  uint16_t m_height;
              public:
                  ConsoleArea(Console* console, uint16_t left, uint16_t top, uint16_t width, uint16_t height);
                  ConsoleArea(Console* console, uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground, ConsoleColour background);
                  ~ConsoleArea();

                  uint16_t width() override;
                  uint16_t height() override;

                  void put_character(uint16_t x, uint16_t y, char c) override;
                  void set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground) override;
                  void set_background_color(uint16_t x, uint16_t y, ConsoleColour background) override;

                  char get_character(uint16_t x, uint16_t y) override;
                  ConsoleColour get_foreground_color(uint16_t x, uint16_t y) override;
                  ConsoleColour get_background_color(uint16_t x, uint16_t y) override;
            };

            /**
             * @class ConsoleStream
             * @brief A stream that can be used to write to a console
             */
            class ConsoleStream : public common::OutputStream
            {
              protected:
                  Console* m_console;

                  ConsoleColour m_foreground;
                  ConsoleColour m_background;

              public:
                  uint16_t m_cursor_x { 0 };
                  uint16_t m_cursor_y { 0 };

                  ConsoleStream(Console*);
                  ~ConsoleStream();

                  void write_char(char c) override;
                  void set_cursor(uint16_t x, uint16_t y);
              };

        }
    }
}

#endif //MAXOS_DRIVERS_CONSOLE_H
