//
// Created by 98max on 10/16/2022.
//

#ifndef MAXOS_GUI_WDIGETS_TEXT_H
#define MAXOS_GUI_WDIGETS_TEXT_H

#include <stdint.h>
#include <common/graphicsContext.h>
#include <gui/widget.h>
#include <gui/font.h>

namespace MaxOS {

    namespace gui {

        namespace widgets{

            /**
             * @class Text
             * @brief A box that can be used to display text
             */
            class Text : public Widget{

            protected:
                string m_widget_text;

            public:

                gui::Font font;

                common::Colour foreground_colour;
                common::Colour background_colour;

                Text(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& text);
                ~Text();

                void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;
                void update_text(const string&);

            };
        }

    }
}

#endif //MAXOS_GUI_WDIGETS_TEXT_H
