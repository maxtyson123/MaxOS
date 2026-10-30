/**
 * @file widget.h
 * @brief Defines a base Widget class and CompositeWidget class for creating graphical user interface elements
 *
 * @date 15th October 2022
 * @author Max Tyson
 */

#ifndef MaxOS_GUI_WIDGET_H
#define MaxOS_GUI_WIDGET_H

#include <stdint.h>
#include <rectangle.h>
#include <vector.h>
#include <common/graphicsContext.h>
#include <coordinates.h>
#include <drivers/peripherals/keyboard.h>
#include <drivers/peripherals/mouse.h>


namespace MaxOS {

	namespace gui {

		/**
		 * @class Widget
		 * @brief A graphical object that can be drawn on the screen
		 */
		class Widget : public drivers::peripherals::KeyboardEventHandler {
				template<int Left, int Top, int Width, int Height> friend class WidgetMoverResizer;

				friend class CompositeWidget;

			protected:
				common::Rectangle<int32_t> m_position;          ///< The position and size of the widget (relative to its parent)

				Widget* m_parent { nullptr };                   ///< The widget that owns this widget
				bool m_valid { false };                         ///< Is the widget drawn to the screen and up to date

				uint32_t m_min_width { 5 };                     ///< The smallest m_width the widget can be resized to
				uint32_t m_min_height { 5 };                    ///< The smallest m_height the widget can be resized to

				uint32_t m_max_width { 0x8FFFFFFF };            ///< The largest m_width the widget can be resized to
				uint32_t m_max_height { 0x8FFFFFFF };           ///< The largest m_height the widget can be resized to

				virtual void set_focus(Widget*);
				virtual void bring_to_front(Widget*);

			public:

				// Initializing functions
				Widget();
				Widget(int32_t left, int32_t top, uint32_t width, uint32_t height);
				~Widget();

				// Drawing functions
				virtual void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);
				void invalidate();
				virtual void invalidate(common::Rectangle<int32_t>& area);
				virtual void add_child(Widget* child);

				// Positioning functions
				virtual common::Coordinates absolute_coordinates(common::Coordinates coordinates);
				virtual bool contains_coordinate(uint32_t x, uint32_t y);
				common::Rectangle<int32_t> position();
				void move(int32_t left, int32_t top);
				void resize(int32_t width, int32_t height);

				// Focus functions
				void focus();
				virtual void on_focus();
				virtual void on_focus_lost();
				void bring_to_front();

				// Mouse functions
				virtual void on_mouse_enter_widget(uint32_t to_x, uint32_t to_y);
				virtual void on_mouse_leave_widget(uint32_t from_x, uint32_t from_y);
				virtual void on_mouse_move_widget(uint32_t from_x, uint32_t from_y, uint32_t to_x, uint32_t to_y);
				virtual drivers::peripherals::MouseEventHandler* on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button);
				virtual void on_mouse_button_released(uint32_t x, uint32_t y, uint8_t button);

		};

		/**
		 * @class CompositeWidget
		 * @brief A widget that can contain other widgets
		 */
		class CompositeWidget : public Widget {

			protected:

				common::Vector<Widget*> m_children;                                                                                     ///< Widgets contained within this composite widget
				void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area, common::Vector<Widget*>::iterator start);
				virtual void draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);


			public:

				// Initializing functions
				CompositeWidget();
				CompositeWidget(int32_t left, int32_t top, uint32_t width, uint32_t height);
				~CompositeWidget();

				// Drawing functions
				void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;
				void add_child(Widget* child) override;

				// Mouse functions
				void on_mouse_enter_widget(uint32_t to_x, uint32_t to_y) override;
				void on_mouse_leave_widget(uint32_t from_x, uint32_t from_y) override;
				void on_mouse_move_widget(uint32_t from_x, uint32_t from_y, uint32_t to_x, uint32_t to_y) override;
				drivers::peripherals::MouseEventHandler* on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button) override;
				void on_mouse_button_released(uint32_t x, uint32_t y, uint8_t button) override;

		};


		/**
		 * @class WidgetMoverResizer
		 * @brief A template class that allows you to move and resize a widget
		 *
		 * @tparam Left Left
		 * @tparam Top Top
		 * @tparam Width Width
		 * @tparam Height Height
		 */
		template<int Left, int Top, int Width, int Height> class WidgetMoverResizer : public drivers::peripherals::MouseEventHandler {
			private:
				Widget* targeted_widget;

			public:
				WidgetMoverResizer(Widget* widget);
				~WidgetMoverResizer();

				void on_mouse_move_event(int8_t x, int8_t y) override;

		};

		typedef WidgetMoverResizer<1, 1, 0, 0> WidgetMover;                         ///< A widget mover that moves the widget without resizing it

		typedef WidgetMoverResizer<0, 1, 0, -1> WidgetMoverResizerTop;              ///< A widget mover and resizer that resizes the top edge of the widget
		typedef WidgetMoverResizer<0, 0, 0, 1> WidgetMoverResizerBottom;            ///< A widget mover andresizer that resizes the bottom edge of  the widget
		typedef WidgetMoverResizer<1, 0, -1, 0> WidgetMoverResizerLeft;             ///< A widget mover and resizer that resizes the left edge of the widget
		typedef WidgetMoverResizer<0, 0, 1, 0> WidgetMoverResizerRight;             ///< A widget mover and resizer that resizes the right edge of the widget

		typedef WidgetMoverResizer<1, 1, -1, -1> WidgetMoverResizerTopLeft;         ///< A widget mover and resizer that resizes the top-left corner of the widget
		typedef WidgetMoverResizer<0, 1, 1, -1> WidgetMoverResizerTopRight;         ///< A widget mover and resizer that resizes the top-right corner of the widget
		typedef WidgetMoverResizer<1, 0, -1, 1> WidgetMoverResizerBottomLeft;       ///< A widget mover and resizer that resizes the bottom-left corner of the widget
		typedef WidgetMoverResizer<0, 0, 1, 1> WidgetMoverResizerBottomRight;       ///< A widget mover and resizer that resizes the bottom-right corner of the widget

		////@TODO move this to the cpp file
		/**
		* @details WidgetMoverResizer is a template class that allows you to move and resize a widget
		*
		* @tparam Left The left edge of the widget
		* @tparam Top The top edge of the widget
		* @tparam Width The m_width of the widget
		* @tparam Height The m_height of the widget
		* @param target The widget to move and resize
		*/
		template<int Left, int Top, int Width, int Height> WidgetMoverResizer<Left, Top, Width, Height>::WidgetMoverResizer(Widget* target)
				: drivers::peripherals::MouseEventHandler() {
			this->targeted_widget = target;
		}

		template<int Left, int Top, int Width, int Height> WidgetMoverResizer<Left, Top, Width, Height>::~WidgetMoverResizer() = default;

		/**
		 * @details OnMouseMoved is called when the mouse is moved. Resizes and moves the widget when this happens.
		 *
		 * @param x The x m_position of the mouse
		 * @param y The y m_position of the mouse
		 */
		template<int Left, int Top, int Width, int Height> void WidgetMoverResizer<Left, Top, Width, Height>::on_mouse_move_event(int8_t x, int8_t y) {

			Widget* targ = this->targeted_widget;

			// If there is actually a size of the widget to change
			if(Left != 0 || Top != 0)
				// Move the widget to the left or right, and up or down
				targ->move(targ->m_position.left + Left * x, targ->m_position.top + Top * y);

			// If there is actually a size of the widget to change
			if(Width != 0 || Height != 0)
				// Resize the widget to the left or right, and up or down
				targ->resize(targ->m_position.width + Width * x, targ->m_position.height + Height * y);
		}
	}
}
#endif //MaxOS_GUI_WIDGET_H
