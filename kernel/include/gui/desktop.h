/**
 * @file desktop.h
 * @brief Defines a Desktop class for managing the GUI desktop, handling mouse and keyboard events, and drawing widgets.
 *
 * @date 15th October 2022
 * @author Max Tyson
 */

#ifndef MaxOS_GUI_DESKTOP_H
#define MaxOS_GUI_DESKTOP_H

#include <cstdint>
#include <common/graphicsContext.h>
#include <drivers/peripherals/mouse.h>
#include <gui/widget.h>
#include <drivers/clock/clock.h>


namespace MaxOS::gui {

	/**
	 * @class Desktop
	 * @brief The desktop that contains all the windows, handles the drawing of the screen and the mouse on every tick
	 *
	 * @todo It is not a good idea to hardcode the mouse into the desktop as a tablet or touch screen device won't have a mouse cursor
	 */
	class Desktop : public CompositeWidget, public drivers::peripherals::MouseEventHandler {

		protected:
			uint32_t m_mouse_x;                                                              ///< The horizontal position of the mouse cursor
			uint32_t m_mouse_y;                                                              ///< The vertical position of the mouse cursor

			common::GraphicsContext* m_graphics_context;                                    ///< Where to draw the desktop to

			Widget* m_focussed_widget { nullptr };                                          ///< The widget that currently is receiving keyboard input and is at the front
			drivers::peripherals::MouseEventHandler* m_dragged_widget { nullptr };          ///< The widget that the mouse is currently dragging

			void set_focus(Widget*) final;
			void bring_to_front(Widget*) final;
			void invert_mouse_cursor();

			common::Vector<common::Rectangle<int32_t>> m_invalid_areas;                    ///< The areas of the desktop that need to be redrawn
			void internal_invalidate(common::Rectangle<int32_t>& area, common::Vector<common::Rectangle<int32_t>>::iterator start, common::Vector<common::Rectangle<int32_t>>::iterator stop);
			void draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) final;

		public:
			common::Colour colour;                                                          ///< The background colour of the desktop (@todo: replace with image, make priv)

			explicit Desktop(common::GraphicsContext* gc);
			~Desktop();

			void add_child(Widget*) final;
			void on_time(const common::Time& time);
			void invalidate(common::Rectangle<int32_t>& area) final;

			// Mouse Events
			void on_mouse_move_event(int8_t x, int8_t y) final;
			void on_mouse_down_event(uint8_t button) final;
			void on_mouse_up_event(uint8_t button) final;

			// Keyboard Events
			void on_key_down(drivers::peripherals::KeyCode key_down_code, const drivers::peripherals::KeyboardState& key_down_state) final;
			void on_key_up(drivers::peripherals::KeyCode key_up_code, const drivers::peripherals::KeyboardState& key_up_state) final;

	};
}


#endif //MaxOS_GUI_DESKTOP_H
