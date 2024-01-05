//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_WINDOW_H
#define MaxOS_GUI_WINDOW_H

#include <stdint.h>
#include <gui/widget.h>
#include <gui/widgets/text.h>

namespace maxOS{

    namespace gui{

        /**
         * @class Window
         * @brief A window that can be moved and resized and contains a widget.
         */
        class Window : public CompositeWidget{

            protected:
                widgets::Text m_title;

                // Resizes
                WidgetMover                   m_mover;
                WidgetMoverResizerTop         m_resizer_top;
                WidgetMoverResizerBottom      m_resizer_bottom;
                WidgetMoverResizerLeft        m_resizer_left;
                WidgetMoverResizerRight       m_resizer_right;
                WidgetMoverResizerTopLeft     m_resizer_top_left;
                WidgetMoverResizerTopRight    m_resizer_top_right;
                WidgetMoverResizerBottomLeft  m_resizer_bottom_left;
                WidgetMoverResizerBottomRight m_resizer_bottom_right;


            public:

                uint8_t frame_thickness { 5 };
                uint8_t title_bar_height { 10 };

                common::Colour area_colour;
                common::Colour frame_colour;
                common::Colour frame_border_colour;

                Window(int32_t left, int32_t top, uint32_t width, uint32_t height, string title_text);
                Window(Widget* containedWidget, string title_text);
                ~Window();

                void draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);
                void add_child(Widget* child);

                drivers::peripherals::MouseEventHandler* on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button);
        };
    }
}
#endif //MaxOS_GUI_WINDOW_H
