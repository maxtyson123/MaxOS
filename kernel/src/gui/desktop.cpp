/**
 * @file desktop.cpp
 * @brief Implementation of a Desktop class for managing the GUI desktop and mouse
 *
 * @date 15th October 2022
 * @author Max Tyson
 *
 * @todo Make a border class
 * @todo Draw a proper mouse cursor
 * @todo Draw a background image
 */

#include <gui/desktop.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::drivers::peripherals;

/**
 * @brief Creates a new desktop with the given graphics context
 *
 * @param gc The graphics context to use
 */
Desktop::Desktop(GraphicsContext* gc)
: CompositeWidget(0, 0, gc->width(), gc->height()),
  MouseEventHandler(),
  m_graphics_context(gc),
  colour(Colour(0xA8, 0xA8, 0xA8))
{

	// Set the mouse m_position to the center of the screen
	m_mouse_x = gc->width() / 2;
	m_mouse_y = gc->height() / 2;

	// Draw the initial mouse cursor
	invert_mouse_cursor();

	// Draw the desktop
	Widget::invalidate();
}

Desktop::~Desktop() = default;

/**
 * @brief Updates the currently focussed widget to be the given widget
 *
 * @param widget The widget that is now in focus
 */
void Desktop::set_focus(Widget* widget) {

	// If there is a widget in focus then send a focus lost event to it
	if (this->m_focussed_widget != nullptr)
		this->m_focussed_widget->on_focus_lost();

	// Focus the new widget and send a focus event to it
	this->m_focussed_widget = widget;
	this->m_focussed_widget->on_focus();
}

/**
 * @brief Brings the given widget to the front of the desktop
 *
 * @param front_widget The widget to bring to the front
 */
void Desktop::bring_to_front(Widget* front_widget) {

	// Remove the widget from where ever it already is
	m_children.erase(front_widget);

	// Add it back in the front
	m_children.push_front(front_widget);
}

/**
 * @brief Draws the mouse cursor at the current mouse m_position by inverting the pixels (mouse is a plus sign)
 */
void Desktop::invert_mouse_cursor() {

	// Draw the horizontal line
	for (int32_t x = m_mouse_x - 3; x <= m_mouse_x + 3; ++x) {
		m_graphics_context->invert_pixel(x, m_mouse_y);
	}

	// Draw the vertical line
	for (int32_t y = m_mouse_y - 3; y <= m_mouse_y + 3; ++y) {
		m_graphics_context->invert_pixel(m_mouse_x, y);
	}
}

/**
 * @brief Goes through the passed areas and invalidates the areas that are covered by the given area
 *
 * @param area The area that is now invalid
 * @param start The start of the invalid areas
 * @param stop The end of the invalid areas
 */
void Desktop::internal_invalidate(common::Rectangle<int32_t>& area, Vector<Rectangle<int32_t>>::iterator start,
								  Vector<Rectangle<int32_t>>::iterator stop) {

	// Loop through the invalid rectangles
	for (Vector<Rectangle<int32_t>>::iterator invaild_rect = start; invaild_rect != stop; invaild_rect++) {

		// Check if the area intersects with the invalid rectangle
		if (!area.intersects(*invaild_rect))
			continue;

		// Get the parts of the area that are covered by the invalid rectangle
		Vector<Rectangle<int32_t>> coveredAreas = area.subtract(*invaild_rect);

		// Invalidate the covered areas
		for (auto& coveredArea: coveredAreas)
			internal_invalidate(coveredArea, invaild_rect + 1, stop);

		// The entire area will be invalidated by now
		return;

	}

	// Add the area to the invalid areas, store where it was added
	Vector<Rectangle<int32_t>>::iterator vectorPosition = m_invalid_areas.push_back(area);

	// If the m_position is the last item then the invalidation buffer is full
	if (vectorPosition == m_invalid_areas.end()) {

		// Invalidate the entire desktop
		m_invalid_areas.clear();
		Widget::invalidate();

	}
}

/**
 * @brief Draws a certain area of the desktop
 *
 * @param gc The graphics context to draw with
 * @param area The area to draw
 */
void Desktop::draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) {

	// Calculate the rectangle
	int32_t topCornerX = area.left;
	int32_t topCornerY = area.top;
	int32_t bottomCornerX = area.left + area.width;
	int32_t bottomCornerY = area.top + area.height;

	// Draw the background, a rectangle the size of the desktop of the given colour
	gc->fill_rectangle(topCornerX, topCornerY, bottomCornerX, bottomCornerY, colour);
}

/**
 * @brief Adds a child widget to the desktop
 *
 * @param child_widget The widget to add
 */
void Desktop::add_child(Widget* child_widget) {

	// Check if the new widget is under the mouse
	bool underMouse = child_widget->contains_coordinate(m_mouse_x, m_mouse_y);

	// If the mouse is over the widget then send a mouse leave event to the child widget as it is no longer under the mouse
	if (underMouse)
		CompositeWidget::on_mouse_leave_widget(m_mouse_x, m_mouse_y);

	// Add the widget to the desktop
	CompositeWidget::add_child(child_widget);

	// If the mouse is over the new widget then send a mouse enter event to the child widget
	if (underMouse)
		CompositeWidget::on_mouse_enter_widget(m_mouse_x, m_mouse_y);
}

/**
 * @brief Redraws the desktop when a time event occurs
 *
 * @param time The time when the event occurred
 */
void Desktop::on_time(common::Time const&) {

	// Check if anything is invalid and needs to be redrawn
	if (m_invalid_areas.empty())
		return;

	// Erase the mouse cursor
	invert_mouse_cursor();

	// Loop through the invalid areas
	while (!m_invalid_areas.empty()) {

		// Redraw the m_first_memory_chunk area
		Rectangle<int32_t> invalidArea = *(m_invalid_areas.begin());
		m_invalid_areas.pop_front();
		draw(m_graphics_context, invalidArea);

	}

	// Can now draw the mouse cursor
	invert_mouse_cursor();

}

/**
 * @brief Invalidate the given area of the desktop
 *
 * @param area The area that is now invalid
 */
void Desktop::invalidate(Rectangle<int32_t>& area) {

	// Invalidate the area
	internal_invalidate(area, m_invalid_areas.begin(), m_invalid_areas.end());

}


/**
 * @brief When the mouse moves on the desktop update the m_position of the mouse and redraw the cursor. Pass the event to the widget that the mouse is over
 *
 * @param x The x m_position of the mouse
 * @param y The y m_position of the mouse
 */
void Desktop::on_mouse_move_event(int8_t x, int8_t y) {

	// Calculate the m_position of the mouse on the desktop
	Rectangle<int32_t> desktopPosition = position();
	int32_t newMouseX = m_mouse_x + x;
	int32_t newMouseY = m_mouse_y + y;

	// Restrain the mouse to the desktop
	if (newMouseX < 0) newMouseX = 0;
	if (newMouseY < 0) newMouseY = 0;
	if (newMouseX > desktopPosition.width) newMouseX = desktopPosition.width - 1;
	if (newMouseY > desktopPosition.height) newMouseY = desktopPosition.height - 1;

	// Remove the old cursor from the screen as it will be redrawn in the new m_position
	invert_mouse_cursor();

	// If a widget is being dragged then pass the event to it
	if (m_dragged_widget != nullptr)
		m_dragged_widget->on_mouse_move_event(newMouseX - m_mouse_x, newMouseY - m_mouse_y);

	// Handle the mouse moving event (pass it to the widget that the mouse is over)
	CompositeWidget::on_mouse_move_widget(m_mouse_x, m_mouse_y, newMouseX,
										  newMouseY);

	// Update the mouse m_position
	m_mouse_x = newMouseX;
	m_mouse_y = newMouseY;

	// Draw the new cursor
	invert_mouse_cursor();
}

/**
 * @brief When the mouse button is pressed pass the event to the widget that the mouse is over
 *
 * @param button The button that was pressed
 */
void Desktop::on_mouse_down_event(uint8_t button) {

	// The widget that handled the event becomes the widget being dragged
	m_dragged_widget = CompositeWidget::on_mouse_button_pressed(m_mouse_x, m_mouse_y, button);
}

/**
 * @brief When the mouse button is released pass the event to the widget that the mouse is over
 *
 * @param button The button that was released
 */
void Desktop::on_mouse_up_event(uint8_t button) {

	// Pass the event to the widget
	CompositeWidget::on_mouse_button_released(m_mouse_x, m_mouse_y, button);

	// Dragging has stopped
	m_dragged_widget = nullptr;
}

/**
 * @brief When a key is pressed pass the event to the widget that is currently focussed
 * @param keyDownCode The key that was pressed
 * @param keyDownState The state of the keyboard
 */
void Desktop::on_key_down(KeyCode keyDownCode, KeyboardState keyDownState) {

	// Pass the event to the widget that is  in focus
	if (m_focussed_widget != nullptr)
		m_focussed_widget->on_key_down(keyDownCode, keyDownState);
}

/**
 * @brief When a key is pressed pass the event to the widget that is currently focussed
 * @param keyUpCode The key that was pressed
 * @param keyUpState The state of the keyboard
 */
void Desktop::on_key_up(KeyCode keyUpCode, KeyboardState keyUpState) {

	// Pass the event to the widget that is  in focus
	if (m_focussed_widget != nullptr)
		m_focussed_widget->on_key_up(keyUpCode, keyUpState);
}