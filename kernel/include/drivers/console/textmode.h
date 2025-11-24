/**
 * @file textmode.h
 * @brief Defines a TextModeConsole class for handling a console in text mode using VGA
 *
 * @date 27th September 2023
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
#define MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H

#include <cstdint>
#include <drivers/driver.h>
#include <drivers/console/console.h>


namespace MaxOS::drivers::console {

	/**
	 * @class TextModeConsole
	 * @brief Driver for the text mode console, handles the printing of characters and strings to the screen using VGA
	 */
	class TextModeConsole : public Driver, public Console {
		private:
			uint16_t* m_video_memory { (uint16_t*) 0xB8000 };

		public:
			TextModeConsole();
			~TextModeConsole();

			uint16_t width() final;
			uint16_t height() final;

			void put_character(uint16_t x, uint16_t y, char c) final;
			void set_foreground_color(uint16_t x, uint16_t y, common::ConsoleColour) final;
			void set_background_color(uint16_t x, uint16_t y, common::ConsoleColour) final;

			char get_character(uint16_t x, uint16_t y) final;
			common::ConsoleColour get_foreground_color(uint16_t x, uint16_t y) final;
			common::ConsoleColour get_background_color(uint16_t x, uint16_t y) final;
	};

}


#endif //MAXOS_DRIVERS_CONSOLE_TEXTMODECONSOLE_H
