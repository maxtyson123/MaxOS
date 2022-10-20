//
// Created by 98max on 10/16/2022.
//

#ifndef MAXOS_GUI_TEXT_H
#define MAXOS_GUI_TEXT_H

#include <common/types.h>
#include <common/graphicsContext.h>
#include <gui/widget.h>

namespace maxOS {

    namespace gui {

        class Text : public CompositeWidget{

            public:

                char* WidgetText;

                Text(Widget *parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b, char* text);
                ~Text();

                void Draw(common::GraphicsContext* gc);
                void UpdateText(char* newText);
        };

    }
}

#endif //MAXOS_GUI_TEXT_H
