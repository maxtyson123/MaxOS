/**
 * @file console.h
 * @brief Defines a Console class for handling text-based console output with colour support
 *
 * @date 27th September 2023
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_CONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_H

#include <cstdint>
#include <outputStream.h>
#include <colour.h>


namespace MaxOS::drivers::console {


	/**
	 * @class Console
	 * @brief Abstract class for a console, allows for the printing of characters and strings with colours
	 */
	class Console {
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
			virtual void put_string(uint16_t x, uint16_t y, string s, common::ConsoleColour foreground = common::ConsoleColour::LightGrey, common::ConsoleColour background = common::ConsoleColour::Black);

			virtual void scroll_up();
			virtual void scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground = common::ConsoleColour::LightGrey, common::ConsoleColour background = common::ConsoleColour::Black, char fill = ' ');

			virtual void clear();
			virtual void clear(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground = common::ConsoleColour::LightGrey, common::ConsoleColour background = common::ConsoleColour::Black, char fill = ' ');

			virtual void invert_colors(uint16_t x, uint16_t y);

	};

	/**
	 * @class ConsoleArea
	 * @brief A console that is a subsection of another console, limited by a width and height
	 */
	class ConsoleArea : public Console {
		private:
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

			void scroll_up() override;
			void scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, common::ConsoleColour foreground = common::ConsoleColour::LightGrey, common::ConsoleColour background = common::ConsoleColour::Black, char fill = ' ') override;

			char get_character(uint16_t x, uint16_t y) override;
			common::ConsoleColour get_foreground_color(uint16_t x, uint16_t y) override;
			common::ConsoleColour get_background_color(uint16_t x, uint16_t y) override;
	};

	/**
	 * @class ConsoleStream
	 * @brief A stream that can be used to write to a console
	 */
	class ConsoleStream : public common::OutputStream {
		private:
			Console* m_console;

			common::ConsoleColour m_foreground { common::ConsoleColour::LightGrey };
			common::ConsoleColour m_background { common::ConsoleColour::Black };
			bool is_ansi = false;

			uint16_t m_cursor_x { 0 };
			uint16_t m_cursor_y { 0 };

		public:

			explicit ConsoleStream(Console*);
			~ConsoleStream();

			void write_char(char c) override;
			void set_cursor(uint16_t x, uint16_t y);
	};

}


#endif //MAXOS_DRIVERS_CONSOLE_H
