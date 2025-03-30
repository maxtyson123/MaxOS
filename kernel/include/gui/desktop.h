//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_GUI_DESKTOP_H
#define MaxOS_GUI_DESKTOP_H

#include <stdint.h>
#include <common/graphicsContext.h>
#include <drivers/peripherals/mouse.h>
#include <gui/widget.h>
#include <drivers/clock/clock.h>

namespace MaxOS{

    namespace gui{

        /**
         * @class Desktop
         * @brief The desktop that contains all the windows, handles the drawing of the screen and the mouse on every tick
         */
        class Desktop : public CompositeWidget, public drivers::peripherals::MouseEventHandler, public drivers::clock::ClockEventHandler{     //NTS: it is not a good idea to hardcode the mouse into the desktop as a tablet or touch screen device won't have a mouse cursor

            protected:
                int32_t m_mouse_x;
                int32_t m_mouse_y;

                common::GraphicsContext* m_graphics_context;

                Widget* m_focussed_widget { nullptr };
                drivers::peripherals::MouseEventHandler* m_dragged_widget { nullptr };

                void set_focus(Widget*) final;
                void bring_to_front(Widget*) final;
                void invert_mouse_cursor();

                common::Vector<common::Rectangle<int32_t> > m_invalid_areas;
                void internal_invalidate(common::Rectangle<int32_t>& area, common::Vector<common::Rectangle<int32_t> >::iterator start, common::Vector<common::Rectangle<int32_t> >::iterator stop);
                void draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) final;

            public:
                common::Colour colour;

                explicit Desktop(common::GraphicsContext* gc);
                ~Desktop();

                void add_child(Widget*) final;
                void on_time(const common::Time& time) final;
                void invalidate(common::Rectangle<int32_t>& area) final;

                // Mouse Events
                void on_mouse_move_event(int8_t x, int8_t y) final;
                void on_mouse_down_event(uint8_t button) final;
                void on_mouse_up_event(uint8_t button) final;

                // Keyboard Events
                void on_key_down(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState) final;
                void on_key_up(drivers::peripherals::KeyCode keyUpCode, drivers::peripherals::KeyboardState keyUpState) final;

        };
    }
}

#endif //MaxOS_GUI_DESKTOP_H
