//
// Created by 98max on 10/15/2022.
//
#include <gui/widget.h>

using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;

///__DEFAULT WIDGET__

Widget::Widget()
: KeyboardEventHandler()
{

}



Widget::Widget(int32_t left, int32_t top, uint32_t width, uint32_t height)
: KeyboardEventHandler(),
  m_position(left, top, width, height)
{

}


Widget::~Widget(){

}

/**
 * @brief Draw the widget on the screen
 *
 * @param gc The graphics context to draw the widgets pixels on
 * @param area The area of the widget to draw
 */
void Widget::draw(GraphicsContext*, Rectangle<int32_t>&) {

}

/**
 * @brief Invalidates the entire widget. This forces the widget to be redrawn on the next screen update
 */
void Widget::invalidate() {

    // Convert the relative coordinates to absolute coordinates
    Coordinates coordinates = absolute_coordinates(Coordinates(0, 0));

    // Create a rectangle with the absolute coordinates and the size of the widget
    Rectangle<int32_t> invalidArea = Rectangle<int32_t>(coordinates.first, coordinates.second, m_position.width, m_position.height);

    // Invalidate the area
    invalidate(invalidArea);

}

/**
 * @brief Invalidates a specific area of the widget. This forces that part of the screen to be redrawn on the next screen update
 *
 * @param area The area of the widget to invalidate
 */
void Widget::invalidate(Rectangle<int32_t> &area) {

    // If the widget has a parent, invalidate the area of the parent
    if(m_parent != 0)
        m_parent->invalidate(area);
}

/**
 * @brief Set the parent of a widget to this widget, making it into a child
 *
 * @param child The child to add
 */
void Widget::add_child(Widget *child) {

    // Parent the child to this widget
    child ->m_parent = this;

}

Coordinates Widget::absolute_coordinates(common::Coordinates coordinates) {

    // Return the parents absolute coordinates
    if(m_parent != 0)
        return m_parent->absolute_coordinates(Coordinates(coordinates.first + m_position.left, coordinates.second + m_position.top));

    // If the widget has no m_parent, return the coordinates of the widget
    return Coordinates(coordinates.first + m_position.left, coordinates.second + m_position.top);

}

/**
 * @brief Check if the widget contains a specific coordinate
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return True if the widget contains the coordinate, false if it does not
 */
bool Widget::contains_coordinate(uint32_t x, uint32_t y) {

    // Check if the coordinates are within the bounds of the widget
    return m_position.contains(x,y);
}

/**
 * @brief Get the position of the widget
 *
 * @return The position of the widget
 */
Rectangle<int32_t> Widget::position() {
    return m_position;
}

/**
 * @brief Set the position of the widget, and invalidate the old and new positions so they are redrawn
 *
 * @param left The new left position of the rectangle
 * @param top The new top position of the rectangle
 */
void Widget::move(int32_t left, int32_t top) {

    // Invalidate the old position
    invalidate();

    // Set the new position
    m_position.left = left;
    m_position.top = top;

    // Re draw the widget in the new position
    invalidate();
}

/**
 * @brief Set the size of the widget, and invalidate the old and new positions so they are redrawn
 *
 * @param width The new m_width of the rectangle
 * @param height The new m_height of the rectangle
 */
void Widget::resize(int32_t width, int32_t height) {

    // Restrict the width and height to the minimum and maximum values
    if(width < (int)m_min_width) width = m_min_width;
    if(height < (int)m_min_height) height = m_min_height;
    if(width > (int)m_max_width) width = m_max_width;
    if(height > (int)m_max_height) height = m_max_height;

    // Store the old position, set the new position
    Rectangle<int32_t> old_position = m_position;
    m_position.width = width;
    m_position.height = height;

    // Find the areas that need to be redrawn by subtracting the old position from the new position, and vice versa
    Vector<Rectangle<int32_t>> invalid_areas_old = old_position.subtract(m_position);
    Vector<Rectangle<int32_t>> invalid_areas_new = m_position.subtract(old_position);

    // Right and Bottom require to be fully invalidated TODO: Fix this hack
    if(m_position.width > old_position.width || m_position.height > old_position.height || old_position.width > m_position.width || old_position.height > m_position.height){
        invalidate();
        return;
    }

    //Loop through the areas that need to be redrawn and invalidate them
    for(uint32_t i = 0; i < invalid_areas_old.size(); i++)
        invalidate(invalid_areas_old[i]);

    for(uint32_t i = 0; i < invalid_areas_new.size(); i++)
        invalidate(invalid_areas_new[i]);

}

/**
 * @brief Set the current focused widget to be this widget
 */
void Widget::focus() {

    // Set the focus the widget to this widget
    set_focus(this);
}

/**
 * @brief Sets the widget that is currently focussed
 *
 * @param widget The widget to set as focussed
 */
void Widget::set_focus(Widget *widget) {

    // Focus the parent to this widget
    if(m_parent != 0)
      m_parent->set_focus(widget);
}

/**
 * @brief Handles the event when the widget is focussed
 */
void Widget::on_focus() {

}

/**
 * @brief Handles the event when the widget is unfocused
 */
void Widget::on_focus_lost() {

}

/**
 * @brief Brings this widget to the front of the screen
 */
void Widget::bring_to_front() {

    // Bring this widget to the front of the screen
    bring_to_front(this);

}

/**
 * @brief Brings a specific widget to the front of the screen
 *
 * @param widget The widget to bring to the front
 */
void Widget::bring_to_front(Widget *widget) {

    // Bring the parent to the front of the screen
    if(m_parent != 0)
       m_parent->bring_to_front(widget);

}

/**
 * @brief Handles the event when the mouse is moved on to the widget
 *
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void Widget::on_mouse_enter_widget(uint32_t, uint32_t) {

}

/**
 * @brief Handles the event when the mouse is moved out of the widget
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 */
void Widget::on_mouse_leave_widget(uint32_t, uint32_t) {

}

/**
 * @brief Handles the event when the mouse is moved over the widget
 *
 * @param fromX The x original coordinate of the mouse
 * @param fromY The y original coordinate of the mouse
 * @param toX The x new coordinate of the mouse
 * @param toY The y new coordinate of the mouse
 */
void Widget::on_mouse_move_widget(uint32_t, uint32_t, uint32_t, uint32_t) {

}

/**
 * @brief Handles the event when the mouse is pressed on the widget
 *
 * @param x The x coordinate of the mouse when it was pressed
 * @param y The y coordinate of the mouse when it was pressed
 * @param button The button that was pressed
 * @return nullptr
 */
peripherals::MouseEventHandler* Widget::on_mouse_button_pressed(uint32_t, uint32_t, uint8_t) {

    // Bring the widget to the front of the screen
    bring_to_front();

    // Focus the widget
    focus();

    // Return 0 as the event has been handled
    return 0;
}

/**
 * @brief Handles the event when the mouse is released on the widget
 *
 * @param x The x coordinate of the mouse when it was released
 * @param y The y coordinate of the mouse when it was released
 * @param button The button that was released
 */
void Widget::on_mouse_button_released(uint32_t, uint32_t, uint8_t) {

}

///__COMPOSITE WIDGET__

CompositeWidget::CompositeWidget() {

}

CompositeWidget::CompositeWidget(int32_t left, int32_t top, uint32_t width, uint32_t height)
: Widget(left, top, width,height)
{

}

CompositeWidget::~CompositeWidget() {

}

/**
 * @brief Draws a section of the widget and its m_children
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 */
void CompositeWidget::draw(GraphicsContext *gc, Rectangle<int32_t> &area) {

    // Draw the widget with its m_children
    draw(gc, area, m_children.begin());

}

/**
 * @brief Draws a section of the widget and the m_children after a specific child
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 * @param start The child to start drawing from
 */
void CompositeWidget::draw(GraphicsContext *gc, Rectangle<int32_t> &area, Vector<Widget *>::iterator start) {

    // Draw the widget
    Widget::draw(gc, area);

    // Get the area of the widget
    Rectangle<int32_t> own_area = position();

    //Note: has to use iterator as the start is not necessarily the m_first_memory_chunk child
    for(Vector<Widget*>::iterator child_widget = start; child_widget != m_children.end(); child_widget++){

        Rectangle<int32_t> child_area = (*child_widget)->position();

        // Check if the child is in the area that needs to be redrawn
        if(area.intersects(child_area)){

            // Get the area that needs to be redrawn
            Rectangle<int32_t> rectangle = area.intersection(child_area);

            // Translate the area so that it is relative to the child
            rectangle.left -= child_area.left;
            rectangle.top -= child_area.top;

            // Draw the child
            (*child_widget)->draw(gc, rectangle);

            // Draw what is left of the area that needs to be redrawn
            Vector<Rectangle<int32_t>> rest_draw_area = area.subtract(child_area);
            for(Vector<Rectangle<int32_t>>::iterator rest_area_part = rest_draw_area.begin(); rest_area_part != rest_draw_area.end(); rest_area_part++)
                draw(gc, *rest_area_part, child_widget + 1);

            // Return as the entire area has now been drawn
            return;
        }
    }

    // Now draw the widget itself
    draw_self(gc, area);
}

/**
 * @brief Draws the widget itself (should be overridden by the derived class)
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 */
void CompositeWidget::draw_self(common::GraphicsContext*, common::Rectangle<int32_t>&) {

}

/**
 * @brief Adds a child to the widget
 *
 * @param child The child to add
 */
void CompositeWidget::add_child(Widget *child) {

        // Store the child and parent the child to this widget
        m_children.push_back(child);
        Widget::add_child(child);
}

/**
 * @brief Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void CompositeWidget::on_mouse_enter_widget(uint32_t toX, uint32_t toY) {

    for(auto&child_widget : m_children){

        // Check if the mouse is in the child
        Rectangle<int32_t> child_area = child_widget->position();
        if(child_area.contains(toX, toY)){

            // Get the position of the mouse relative to the child
            uint32_t child_x = toX - child_area.left;
            uint32_t child_y = toY - child_area.top;

            // Call the child's on_mouse_enter_widget function
            child_widget->on_mouse_enter_widget(child_x, child_y);

            // Break as the event has been handled
            break;
        }
    }

}
/**
 * @brief Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 */
void CompositeWidget::on_mouse_leave_widget(uint32_t fromX, uint32_t fromY) {

    for(auto&child_widget : m_children){

        // Check if the mouse is in the child
        Rectangle<int32_t> child_area = child_widget->position();
        if(child_area.contains(fromX, fromY)){

            // Get the position of the mouse relative to the child
            uint32_t child_x = fromX - child_area.left;
            uint32_t child_y = fromY - child_area.top;

            // Call the child's on_mouse_leave_widget function
            child_widget->on_mouse_leave_widget(child_x, child_y);

            // Event has been handled
            break;
        }
    }
}

/**
 * @brief Passes the event to the child that the mouse is over, also generates a leave/enter event for children
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void CompositeWidget::on_mouse_move_widget(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY) {

    Widget* left_child = nullptr;
    Widget* entered_child = nullptr;

    for(auto&child_widget : m_children){

        // Check if the mouse is in the child
        Rectangle<int32_t> child_area = child_widget->position();
        bool mouse_in_from = child_area.contains(fromX, fromY);
        bool mouse_in_to = child_area.contains(toX, toY);

        // If the mouse started in the child
        if(mouse_in_from){

            // The mouse moved out of the child
            if(!mouse_in_to){
                left_child = child_widget;
                continue;
            }

            // Mouse still in the child
            child_widget->on_mouse_move_widget(fromX, fromY, toX, toY);

        }else{

            // Mouse moved into the child
            if(mouse_in_to)
                entered_child = child_widget;
        }

        // Pass the events to the child
        if(left_child != nullptr)
          left_child->on_mouse_leave_widget(fromX, fromY);

        if(entered_child != nullptr)
          entered_child->on_mouse_enter_widget(toX, toY);
    }
}

/**
 * @brief Passes the event to the child that the mouse is over.
 *
 * @param x The x coordinate of the mouse
 * @param y The y coordinate of the mouse
 * @param button The button that was pressed
 * @return The object that has the mouseEventHandler which handled the event
 */
peripherals::MouseEventHandler *CompositeWidget::on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button) {

    MouseEventHandler*mouse_event_handler = 0;

    for(auto&child_widget : m_children){

        // Pass the event to the child
        if(child_widget->contains_coordinate(x, y)){
            mouse_event_handler = child_widget -> on_mouse_button_pressed(x - child_widget->m_position.left, y - child_widget->m_position.top, button);
            break;
        }

    }

    return mouse_event_handler;

}

/**
 * @brief Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param x The x coordinate of the mouse
 * @param y The y coordinate of the mouse
 * @param button The button that was released
 */
void CompositeWidget::on_mouse_button_released(uint32_t x, uint32_t y, uint8_t button) {

    // Loop through the m_children
    for(auto&child_widget : m_children){

        // Pass the event to the child
        if(child_widget->contains_coordinate(x, y)){
            child_widget->on_mouse_button_released(x - child_widget->m_position.left, y - child_widget->m_position.top, button);
            break;
        }
    }
}
