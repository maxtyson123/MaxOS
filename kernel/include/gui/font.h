//
// Created by 98max on 4/15/2023.
//

#ifndef MAXOS_COMMON_FONT_H
#define MAXOS_COMMON_FONT_H

#include <stdint.h>
#include<common/graphicsContext.h>
#include<common/rectangle.h>
#include <common/string.h>

namespace maxOS{

    namespace gui{

        /**
         * @class Font
         * @brief A class that can be used to draw text
         */
        class Font{
            protected:
                bool m_is_8_by_8;

            public:
                uint16_t font_size { 8 };

                bool is_bold { false };
                bool is_italic { false };
                bool is_underlined { false };
                bool is_strikethrough { false };

                Font();
                ~Font();

                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text);
                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text, common::Rectangle<int32_t> limitArea);

                virtual void get_font_8_x_8(uint8_t (&font8x8)[2048]);

                virtual uint32_t get_text_height(string);
                virtual uint32_t get_text_width(string);

        };

        /**
         * @class AmigaFont
         * @brief A font that uses the Amiga 8x8 font
         */
        class AmigaFont : public Font{
            public:
                AmigaFont();
                ~AmigaFont();

                virtual void get_font_8_x_8(uint8_t (&font8x8)[2048]) final;

        };
    }

}

#endif //MAXOS_COMMON_FONT_H
