//
// Created by 98max on 10/16/2022.
//

#ifndef MAXOS_GUI_WDIGETS_TEXT_H
#define MAXOS_GUI_WDIGETS_TEXT_H

#include <common/types.h>
#include <common/graphicsContext.h>
#include <gui/widget.h>
#include <gui/font.h>

namespace maxOS {

    namespace gui {

        namespace widgets{
            class Text : public Widget{

            protected:
                char widgetText[256];

            public:

                //TODO: Redo so that it uses some generified font class
                AmigaFont font;

                common::Colour foregroundColour;
                common::Colour backgroundColour;

                Text(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height, common::string text);
                ~Text();

                void draw(common::GraphicsContext* gc, common::Rectangle<int>& area);
                void updateText(common::string newText);

            };
        }

    }
}

#endif //MAXOS_GUI_WDIGETS_TEXT_H
