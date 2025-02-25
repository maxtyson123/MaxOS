//
// Created by 98max on 27/09/2023.
//

#ifndef MAXOS_DRIVERS_CONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_H

#include <stdint.h>
#include <common/outputStream.h>
#include <common/colour.h>

namespace MaxOS {

    namespace drivers {

        namespace console {



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

                  virtual void set_foreground_color(uint16_t x, uint16_t y, common::ConsoleColour foreground);
                  virtual void set_background_color(uint16_t x, uint16_t y, common::ConsoleColour background);

                  virtual common::ConsoleColour get_foreground_color(uint16_t x, uint16_t y);
                  virtual common::ConsoleColour get_background_color(uint16_t x, uint16_t y);


                  virtual void put_character(uint16_t x, uint16_t y, char c, common::ConsoleColour foreground, common::ConsoleColour background);
                  virtual void put_string(uint16_t x, uint16_t y, string s, common::ConsoleColour foreground = common::LightGrey, common::ConsoleColour background = common::Black);

                  virtual void scroll_up();
                  virtual void scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground = common::LightGrey, common::ConsoleColour background = common::Black, char fill= ' ');

                  virtual void clear();
                  virtual void clear(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground = common::LightGrey, common::ConsoleColour background = common::Black, char fill=' ');

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
                  ConsoleArea(Console* console, uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground, common::ConsoleColour background);
                  ~ConsoleArea();

                  uint16_t width() override;
                  uint16_t height() override;

                  void put_character(uint16_t x, uint16_t y, char c) override;
                  void set_foreground_color(uint16_t x, uint16_t y, common::ConsoleColour foreground) override;
                  void set_background_color(uint16_t x, uint16_t y, common::ConsoleColour background) override;

                  char get_character(uint16_t x, uint16_t y) override;
                  common::ConsoleColour get_foreground_color(uint16_t x, uint16_t y) override;
                  common::ConsoleColour get_background_color(uint16_t x, uint16_t y) override;
            };

            /**
             * @class ConsoleStream
             * @brief A stream that can be used to write to a console
             */
            class ConsoleStream : public common::OutputStream
            {
              protected:
                  Console* m_console;

                  common::ConsoleColour m_foreground;
                  common::ConsoleColour m_background;
                  bool is_ansi = false;

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
