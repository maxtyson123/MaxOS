/**
 * @file window.h
 * @brief Defines a Window class for creating movable and resizable GUI windows
 *
 * @date 15th October 2022
 * @author Max Tyson
 */

#ifndef MaxOS_GUI_WINDOW_H
#define MaxOS_GUI_WINDOW_H

#include <stdint.h>
#include <gui/widget.h>
#include <gui/widgets/text.h>

namespace MaxOS{

    namespace gui{

        /**
         * @class Window
         * @brief A window that can be moved and resized and contains a widget.
         */
        class Window : public CompositeWidget{

	        private:
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

                uint8_t frame_thickness { 5 };              ///< How thick the bar around the window is
                uint8_t title_bar_height { 10 };            ///< How large the bar at the top of the window is

                common::Colour area_colour;                 ///< The background colour of the window inner area
                common::Colour frame_colour;                ///< The colour of the bar around the window
                common::Colour frame_border_colour;         ///< The colour of the border around the bar around the window

                Window(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& title_text);
                Window(Widget* contained_widget, const string& title_text);
                ~Window();

                void draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;
                void add_child(Widget* child) override;

                drivers::peripherals::MouseEventHandler* on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button) override;
        };
    }
}
#endif //MaxOS_GUI_WINDOW_H
