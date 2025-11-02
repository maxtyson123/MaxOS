/**
 * @file window.cpp
 * @brief Implementation of a Window class for GUI windows
 *
 * @date 15th October 2022
 * @author Max Tyson
 */

#include <gui/window.h>
#include <gui/widgets/text.h>

using namespace MaxOS;
using namespace MaxOS::gui;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;

/**
 * @brief Construct a new Window object at a specific position and size (relative to the screen) with a title
 *
 * @param left How many pixels from the left of the screen the window is
 * @param top How many pixels from the top of the screen the window is
 * @param width How many pixels wide the window is
 * @param height How many pixels tall the window is
 * @param title_text The text to display in the title bar of the window
 */
Window::Window(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& title_text)
: CompositeWidget(left, top, width, height),
  m_title(0, -(10 + 5) + 2, width - 2 * 5, 10 + 5 - 3, title_text),
  m_mover(this),
  m_resizer_top(this),
  m_resizer_bottom(this),
  m_resizer_left(this),
  m_resizer_right(this),
  m_resizer_top_left(this),
  m_resizer_top_right(this),
  m_resizer_bottom_left(this),
  m_resizer_bottom_right(this)
{

	// Set the sizing
	m_min_width = 2 * frame_thickness;
	m_min_height = 2 * frame_thickness + title_bar_height;

	// Set the colours
	area_colour = Colour(0xff, 0xff, 0xff);    // White
	frame_border_colour = Colour(0x00, 0x00, 0x00);    // Black
	frame_colour = Colour(0x57, 0x57, 0x57);      // Davy's Grey
	m_title.foreground_colour = Colour(0xff, 0xff, 0xff);    // White
	m_title.background_colour = frame_colour;

	// Add the title to the window
	Window::add_child(&m_title);

}

/**
 * @brief Construct a new Window object that contains a specific widget with a title. The window sizes itself to fit the widget.
 *
 * @param containedWidget The widget to contain within the window
 * @param title_text The text to display in the title bar of the window
 */
Window::Window(Widget* containedWidget, const string& title_text)
: CompositeWidget(0, 0, containedWidget->position().width + 2 * 5 + 2, containedWidget->position().height + 2 * 5 + 10 + 2),
  m_title(0, -(10 + 5) + 2, containedWidget->position().width, 10 + 5 - 3, title_text),
  m_mover(this),
  m_resizer_top(this),
  m_resizer_bottom(this),
  m_resizer_left(this),
  m_resizer_right(this),
  m_resizer_top_left(this),
  m_resizer_top_right(this),
  m_resizer_bottom_left(this),
  m_resizer_bottom_right(this)
{

	// Set the sizing
	m_min_width = 2 * frame_thickness;
	m_min_height = 2 * frame_thickness + title_bar_height;

	// Set the colours
	area_colour = Colour(0xff, 0xff, 0xff);    // White
	frame_border_colour = Colour(0x00, 0x00, 0x00);    // Black
	frame_colour = Colour(0x57, 0x57, 0x57);      // Davy's Grey
	m_title.foreground_colour = Colour(0xff, 0xff, 0xff);    // White
	m_title.background_colour = frame_colour;

	// Add the title to the window
	Window::add_child(&m_title);
	Window::add_child(containedWidget);
}

Window::~Window() = default;

/**
 * @brief Handles the mouse button being pressed.
 *
 * @param mouse_x The x coordinate of the mouse.
 * @param mouse_y The y coordinate of the mouse.
 * @param button The button that is pressed.
 * @return The MouseEventHandler of the widget that will handle the event, nullptr otherwise.
 */
MouseEventHandler* Window::on_mouse_button_pressed(uint32_t mouse_x, uint32_t mouse_y, uint8_t button) {

	// Pass the mouse event to the children
	drivers::peripherals::MouseEventHandler* child_result = CompositeWidget::on_mouse_button_pressed(mouse_x, mouse_y, button);
	Rectangle<int32_t> window_position = position();

	// Bring the window to the front
	bring_to_front();

	// Convert the mouse coordinates to an int32_t
	auto x = (int32_t) mouse_x;
	auto y = (int32_t) mouse_y;

	if (x <= frame_thickness) {
		if (y <= frame_thickness)
			return &m_resizer_top_left;

		else if (y < window_position.height - frame_thickness)
			return &m_resizer_left;

		else
			return &m_resizer_bottom_left;
	} else if (x < window_position.width - frame_thickness) {
		if (y <= frame_thickness)
			return &m_resizer_top;

		else if (y < frame_thickness + title_bar_height)
			return &m_mover;

		else if (y >= window_position.height - frame_thickness)
			return &m_resizer_bottom;
	} else {
		if (y <= frame_thickness)
			return &m_resizer_top_right;

		else if (y < window_position.height - frame_thickness)
			return &m_resizer_right;

		else
			return &m_resizer_bottom_right;
	}

	return child_result;
}

/**
 * @brief Draws the window and its children.
 *
 * @param gc The graphics context to draw on.
 * @param area The area to draw
 */
void Window::draw_self(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) {

	// Get the positioning of the window
	Coordinates window_absolute_position = CompositeWidget::absolute_coordinates(Coordinates(0, 0));
	Rectangle<int32_t> windowPosition = this->position();
	int32_t window_x = window_absolute_position.first;
	int32_t window_y = window_absolute_position.second;

	// Create an area for the window contents
	Rectangle<int32_t> window_contents_area(frame_thickness, frame_thickness + title_bar_height,
											windowPosition.width - 2 * frame_thickness,
											windowPosition.height - 2 * frame_thickness - title_bar_height);

	// Draw the window contents if they are in the area to draw
	if (window_contents_area.intersects(area)) {
		Rectangle<int32_t> contents_drawable = window_contents_area.intersection(area);
		gc->fill_rectangle(contents_drawable.left + window_x, contents_drawable.top + window_y,
						   contents_drawable.left + contents_drawable.width + window_x,
						   contents_drawable.top + contents_drawable.height + window_y, area_colour);
	}

	// Draw the frame if it is in the area to draw
	Rectangle<int32_t> window_frame_top_area(frame_thickness, 0, windowPosition.width - 2 * frame_thickness,
											 frame_thickness + title_bar_height);
	if (window_frame_top_area.intersects(area)) {
		Rectangle<int32_t> frame_drawable = window_frame_top_area.intersection(area);
		gc->fill_rectangle(frame_drawable.left + window_x, frame_drawable.top + window_y,
						   frame_drawable.left + frame_drawable.width + window_x,
						   frame_drawable.top + frame_drawable.height + window_y, frame_colour);
	}

	// Draw the bottom of the window frame
	Rectangle<int32_t> window_frame_bottom_area(frame_thickness, windowPosition.height - frame_thickness,
												windowPosition.width - 2 * frame_thickness, frame_thickness);
	if (window_frame_bottom_area.intersects(area)) {
		Rectangle<int32_t> bottom_drawable = window_frame_bottom_area.intersection(area);
		gc->fill_rectangle(window_x + bottom_drawable.left, window_y + bottom_drawable.top,
						   window_x + bottom_drawable.left + bottom_drawable.width,
						   window_y + bottom_drawable.top + bottom_drawable.height, frame_colour);
	}

	// Draw the left of the window frame
	Rectangle<int32_t> window_frame_left_area(0, 0, frame_thickness, windowPosition.height);
	if (window_frame_left_area.intersects(area)) {
		Rectangle<int32_t> left_drawable = window_frame_left_area.intersection(area);
		gc->fill_rectangle(window_x + left_drawable.left, window_y + left_drawable.top,
						   window_x + left_drawable.left + left_drawable.width,
						   window_y + left_drawable.top + left_drawable.height, frame_colour);
	}

	// Draw the right of the window frame
	Rectangle<int32_t> window_frame_right_area(windowPosition.width - frame_thickness, 0, frame_thickness,
											   windowPosition.height);
	if (window_frame_right_area.intersects(area)) {
		Rectangle<int32_t> right_drawable = window_frame_right_area.intersection(area);
		gc->fill_rectangle(window_x + right_drawable.left, window_y + right_drawable.top,
						   window_x + right_drawable.left + right_drawable.width,
						   window_y + right_drawable.top + right_drawable.height, frame_colour);
	}
}

/**
 * @brief Adds a child to the window.
 *
 * @param child The child to add.
 */
void Window::add_child(Widget* child) {

	// If there is a child to add
	if (child != nullptr) {

		// Change the position of the child to be inside the window contents
		Rectangle<int32_t> childPosition = child->position();
		child->move(childPosition.left + frame_thickness + 1,
					childPosition.top + frame_thickness + title_bar_height + 1);

	}

	// Add the child to the window
	CompositeWidget::add_child(child);
}