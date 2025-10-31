//
// Created by 98max on 4/15/2023.
//

#ifndef MAXOS_COMMON_FONT_H
#define MAXOS_COMMON_FONT_H

#include <stdint.h>
#include<common/graphicsContext.h>
#include<common/rectangle.h>
#include <common/string.h>

namespace MaxOS{

    namespace gui{

		constexpr uint8_t FONT_PADDING = 3;                         ///< How many vertical pixels to add to the top and bottom of the font to prevent squishing
		constexpr uint8_t FONT_HEIGHT = 8 + (2 * FONT_PADDING);     ///< How many pixels tall the font is
		constexpr uint8_t FONT_WIDTH = 8;                           ///< How many pixels wide the font is

        /**
         * @class Font
         * @brief A class that can be used to draw text
         */
        class Font{
            protected:
                bool m_is_8_by_8 = { true };                ///< Is the font 8 pixels by 8 pixels per character
                uint8_t m_font8x8[2048] = {0};              ///< The 8x8 font data

            public:

                Font(const uint8_t* font_data);
                ~Font();

                bool is_bold { false };                     ///< Should the font be drawn in bold
                bool is_italic { false };                   ///< Should the font be drawn in italic
                bool is_underlined { false };               ///< Should the font be drawn with an underline
                bool is_strikethrough { false };            ///< Should the font be drawn with a strikethrough

                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text);
                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text, common::Rectangle<int32_t> limitArea);

                virtual int32_t get_text_height(string);
                virtual int32_t get_text_width(string);
        };
    }

}

#endif //MAXOS_COMMON_FONT_H
