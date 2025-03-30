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

        /**
         * @class Font
         * @brief A class that can be used to draw text
         */
        class Font{
            protected:
                bool m_is_8_by_8 = { true };
                uint8_t m_font8x8[2048] = {0};

            public:

                // Font takes the data
                Font(const uint8_t* font_data);
                ~Font();

                uint16_t font_size { 8 };

                bool is_bold { false };
                bool is_italic { false };
                bool is_underlined { false };
                bool is_strikethrough { false };

                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text);
                virtual void draw_text(int32_t x, int32_t y, common::Colour foreground_colour, common::Colour background_colour, common::GraphicsContext *context, string text, common::Rectangle<int32_t> limitArea);

                virtual int32_t get_text_height(string);
                virtual int32_t get_text_width(string);

        };
    }

}

#endif //MAXOS_COMMON_FONT_H
