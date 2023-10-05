//
// Created by 98max on 4/15/2023.
//

#ifndef MAXOS_COMMON_FONT_H
#define MAXOS_COMMON_FONT_H

#include<common/types.h>
#include<common/graphicsContext.h>
#include<common/rectangle.h>

namespace maxOS{

    namespace gui{

        class Font{
            protected:
                bool is8x8;

            public:
                common::uint16_t fontSize;
                bool isBold;

                bool isItalic;
                bool isUnderlined;
                bool isStrikethrough;

                Font();
                ~Font();

                virtual void drawText(common::int32_t x, common::int32_t y, common::Colour foregroundColour, common::Colour backgroundColour, common::GraphicsContext* context, common::string text);
                virtual void drawText(common::int32_t x, common::int32_t y, common::Colour foregroundColour, common::Colour backgroundColour, common::GraphicsContext* context, common::string text, common::Rectangle<int> limitArea);

                virtual void getFont8x8(common::uint8_t (&font8x8)[2048]);

                virtual common::uint32_t getTextHeight(common::string text);
                virtual common::uint32_t getTextWidth(common::string text);

        };


        class AmigaFont : public Font{
            public:
                AmigaFont();
                ~AmigaFont();

                virtual void getFont8x8(common::uint8_t (&font8x8)[2048]);

        };
    }

}

#endif //MAXOS_COMMON_FONT_H
