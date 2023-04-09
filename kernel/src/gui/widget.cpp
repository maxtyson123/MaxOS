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

    // No parent, not valid
    parent = 0;
    valid = false;

    // Set the minimum size to 5px
    minWidth = 5;
    minHeight = 5;

    // Set tne maximum size to the maximum value of a 32 bit integer
    maxWidth = 0x8FFFFFFF;
    maxHeight = 0x8FFFFFFF;

}

Widget::Widget(int32_t left, int32_t top, uint32_t width, uint32_t height)
: KeyboardEventHandler(),
  position(left, top, width, height)
{

    // No parent, not valid
    parent = 0;
    valid = false;

    // Set the minimum size to 5px
    minWidth = 5;
    minHeight = 5;

    // Set tne maximum size to the maximum value of a 32 bit integer
    maxWidth = 0x8FFFFFFF;
    maxHeight = 0x8FFFFFFF;

}


Widget::~Widget(){

}

/**
 * @details Draw the widget on the screen
 *
 * @param gc The graphics context to draw the widgets pixels on
 * @param area The area of the widget to draw
 */
void Widget::draw(GraphicsContext *gc, Rectangle<int> &area) {

}

/**
 * @details Invalidates the entire widget. This forces the widget to be redrawn on the next screen update
 */
void Widget::invalidate() {

    // Convert the relative coordinates to absolute coordinates
    Coordinates absCoords = absoluteCoordinates(Coordinates(0,0));

    // Create a rectangle with the absolute coordinates and the size of the widget
    Rectangle<int> invalidArea = Rectangle<int>(absCoords.first, absCoords.second, position.width, position.height);

    // Invalidate the area
    invalidate(invalidArea);

}

/**
 * @details Invalidates a specific area of the widget. This forces that part of the screen to be redrawn on the next screen update
 *
 * @param area The area of the widget to invalidate
 */
void Widget::invalidate(Rectangle<int> &area) {

        // If the widget has a parent, invalidate the area relative to the parent
        if(parent != 0){
            parent -> invalidate(area);
        }

}

/**
 * @details Set the parent of a widget to this widget, making it into a child
 *
 * @param child The child to add
 */
void Widget::addChild(Widget *child) {

    // Set the parent of the child to this widget
    child -> parent = this;

}

Coordinates Widget::absoluteCoordinates(Coordinates coordinates) {

    // If the widget has a parent, add the coordinates of the parent to the coordinates of the widget
    if(parent != 0){
        return parent -> absoluteCoordinates(Coordinates(coordinates.first + position.left, coordinates.second + position.top));
    }

    // If the widget has no parent, return the coordinates of the widget
    return Coordinates(coordinates.first + position.left, coordinates.second + position.top);

}

/**
 * @details Check if the widget contains a specific coordinate
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return True if the widget contains the coordinate, false if it does not
 */
bool Widget::containsCoordinate(uint32_t x, uint32_t y) {

    // Check if the coordinates are within the bounds of the widget
    return position.contains(x,y);
}

/**
 * @details Get the position of the widget
 *
 * @return The position of the widget
 */
Rectangle<int> Widget::getPosition() {
    return position;
}

/**
 * @details Set the position of the widget, and invalidate the old and new positions so they are redrawn
 *
 * @param left The new left position of the rectangle
 * @param top The new top position of the rectangle
 */
void Widget::move(int32_t left, int32_t top) {
    // Widget is no longer valid as it needs to be redrawn somewhere else. This means the old position needs to be invalidated so the pixels behind this widget are redrawn
    invalidate();

    // Set the new position
    position.left = left;
    position.top = top;

    // Widget is now valid so update the pixels in the new position
    invalidate();
}

/**
 * @details Set the size of the widget, and invalidate the old and new positions so they are redrawn
 *
 * @param width The new width of the rectangle
 * @param height The new height of the rectangle
 */
void Widget::resize(int32_t width, int32_t height) {

    // Restrict the width and height to the minimum and maximum values
    if(width < minWidth) width = minWidth;
    if(height < minHeight) height = minHeight;
    if(width > maxWidth) width = maxWidth;
    if(height > maxHeight) height = maxHeight;

    // Store the old position, set the new position
    Rectangle<int> oldPosition = position;
    position.width = width;
    position.height = height;

    // Find the areas that need to be redrawn by subtracting the old position from the new position, and vice versa
    Vector<Rectangle<int>> invalidAreasOld = oldPosition.subtract(position);
    Vector<Rectangle<int>> invalidAreasNew = position.subtract(oldPosition);

    //Loop through the areas that need to be redrawn and invalidate them
    for(int i = 0; i < invalidAreasOld.size(); i++){
        invalidate(invalidAreasOld[i]);
    }

    for(int i = 0; i < invalidAreasNew.size(); i++){
        invalidate(invalidAreasNew[i]);
    }
}

/**
 * @details Set the current focuesd widget to be this widget
 */
void Widget::focus() {
    // Set the focus the widget to this widget
    setFocus(this);
}

/**
 * @details Sets the widget that is currently focussed
 * @param widget The widget to set as focussed
 */
void Widget::setFocus(Widget *widget) {

    // If the widget has a parent, set the focus of the parent to this widget
    if(parent != 0){
        parent->setFocus(widget);
    }
}

/**
 * @details Handles the event when the widget is focussed
 */
void Widget::onFocus() {

}

/**
 * @details Handles the event when the widget is unfocussed
 */
void Widget::onFocusLost() {

}

/**
 * @details Brings this widget to the front of the screen
 */
void Widget::bringToFront() {

}

/**
 * @details Brings a specific widget to the front of the screen
 *
 * @param widget The widget to bring to the front
 */
void Widget::bringToFront(Widget *widget) {

    // If the widget has a parent, bring the widget to the front of the parent
    if(parent != 0){
        parent->bringToFront(widget);
    }

}

/**
 * @details Handles the event when the mouse is moved on to the widget
 *
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void Widget::onMouseEnter(uint32_t toX, uint32_t toY) {

}

/**
 * @details Handles the event when the mouse is moved out of the widget
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 */
void Widget::onMouseLeave(uint32_t fromX, uint32_t fromY) {

}

/**
 * @details Handles the event when the mouse is moved over the widget
 *
 * @param fromX The x original coordinate of the mouse
 * @param fromY The y original coordinate of the mouse
 * @param toX The x new coordinate of the mouse
 * @param toY The y new coordinate of the mouse
 */
void Widget::onMouseMove(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY) {

}

/**
 * @details Handles the event when the mouse is pressed on the widget
 *
 * @param x The x coordinate of the mouse when it was pressed
 * @param y The y coordinate of the mouse when it was pressed
 * @param button The button that was pressed
 * @return nullptr
 */
peripherals::MouseEventHandler* Widget::onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button) {
    // Bring the widget to the front of the screen
    bringToFront();

    // Focus the widget
    focus();

    // Return 0 as the event has been handled
    return 0;
}

/**
 * @details Handles the event when the mouse is released on the widget
 *
 * @param x The x coordinate of the mouse when it was released
 * @param y The y coordinate of the mouse when it was released
 * @param button The button that was released
 */
void Widget::onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button) {

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
 * @details Draws a section of the widget and its children
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 */
void CompositeWidget::draw(GraphicsContext *gc, Rectangle<int> &area) {

    // Draw the widget with its children
    draw(gc, area, children.begin());

}

/**
 * @details Draws a section of the widget and the children after a specific child
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 * @param start The child to start drawing from
 */
void CompositeWidget::draw(GraphicsContext *gc, Rectangle<int> &area, Vector<Widget *>::iterator start) {

    // Draw the widget
    Widget::draw(gc, area);

    // Get the comp widgets own are
    Rectangle<int> ownArea = getPosition();

    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = start; childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the child is in the area that needs to be redrawn
        if(area.intersects(childArea)){

            // Get the area that needs to be redrawn
            Rectangle<int> redrawArea = area.intersection(childArea);

            // Translate the area so that it is relative to the child
            redrawArea.left -= childArea.left;
            redrawArea.top -= childArea.top;

            // Draw the child
            (*childWidget)->draw(gc, redrawArea);

            // Get what is left to draw (the area that is not covered by the child)
            Vector<Rectangle<int>> restDrawArea = area.subtract(childArea);

            // Loop through the areas that need to be redrawn
            for(Vector<Rectangle<int>>::iterator restAreaPart = restDrawArea.begin(); restAreaPart != restDrawArea.end(); restAreaPart++){

                // Call the draw function again with the part of the area, starting from the next child as the children up to this point have already been drawn
                draw(gc, *restAreaPart, childWidget + 1);

            }

            // Return as the entire area has now been drawn
            return;
        }
    }

    // Now draw the widget itself
    drawSelf(gc, area);
}

/**
 * @details Draws the widget itself (should be overridden by the derived class)
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw
 */
void CompositeWidget::drawSelf(GraphicsContext *gc, Rectangle<int> &area) {

}

/**
 * @details Adds a child to the widget
 *
 * @param child The child to add
 */
void CompositeWidget::addChild(Widget *child) {

        // Add the child to the list of children
        children.push_back(child);

        // Set the parent of the child to this widget
        Widget::addChild(child);
}

/**
 * @details Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void CompositeWidget::onMouseEnter(uint32_t toX, uint32_t toY) {

    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = children.begin(); childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the mouse is in the child
        if(childArea.contains(toX, toY)){

            // Get the position of the mouse relative to the child
            uint32_t childX = toX - childArea.left;
            uint32_t childY = toY - childArea.top;

            // Call the child's onMouseEnter function
            (*childWidget)->onMouseEnter(childX, childY);

            // Break as the event has been handled
            break;
        }
    }

}
/**
 * @details Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 */
void CompositeWidget::onMouseLeave(uint32_t fromX, uint32_t fromY) {
    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = children.begin(); childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the mouse is in the child
        if(childArea.contains(fromX, fromY)){

            // Get the position of the mouse relative to the child
            uint32_t childX = fromX - childArea.left;
            uint32_t childY = fromY - childArea.top;

            // Call the child's onMouseLeave function
            (*childWidget)->onMouseLeave(childX, childY);

            // Break as the event has been handled
            break;
        }
    }
}

/**
 * @details Passes the event to the child that the mouse is over, also generates a leave/enter event for children (Event handling should be done by the derived class)
 *
 * @param fromX The x coordinate of the mouse
 * @param fromY The y coordinate of the mouse
 * @param toX The x coordinate of the mouse
 * @param toY The y coordinate of the mouse
 */
void CompositeWidget::onMouseMove(uint32_t fromX, uint32_t fromY, uint32_t toX, uint32_t toY) {

    Widget* leftChild = nullptr;
    Widget* enteredChild = nullptr;

    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = children.begin(); childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the mouse is in the child
        bool mouseInFrom = childArea.contains(fromX, fromY);
        bool mouseInTo = childArea.contains(toX, toY);

        // If the mouse started in the child
        if(mouseInFrom){

            // Check if the mouse moved out of the child
            if(!mouseInTo){

                // The mouse started in the child but is now outside it so it must have left the child
                leftChild = *childWidget;
                continue;
            }

            // This means the mouse is still in the child so pass the event to the child
            (*childWidget)->onMouseMove(fromX, fromY, toX, toY); //TODO: Use relative? (*childWidget)->onMouseMove(fromX - childArea.left, fromY - childArea.top, toX - childArea.left, toY - childArea.top

        }else{

            // Check if the mouse moved into the child
            if(mouseInTo){
                // The mouse started outside the child but is now in it so it must have entered the child
                enteredChild = *childWidget;
            }

        }

        // Pass the events to the children
        if(leftChild != nullptr){
            leftChild->onMouseLeave(fromX, fromY);
        }

        if(enteredChild != nullptr){
            enteredChild->onMouseEnter(toX, toY);
        }
    }
}

/**
 * @details Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param x The x coordinate of the mouse
 * @param y The y coordinate of the mouse
 * @param button The button that was pressed
 * @return The handler for the event
 */
peripherals::MouseEventHandler *CompositeWidget::onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button) {

    MouseEventHandler* handler = nullptr;

    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = children.begin(); childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the mouse is in the child
        if(childArea.contains(x, y)){

            // Get the position of the mouse relative to the child
            uint32_t childX = x - childArea.left;
            uint32_t childY = y - childArea.top;

            // Call the child's onMouseButtonPressed function
            handler = (*childWidget)->onMouseButtonPressed(childX, childY, button);

            // Break as the event has been handled
            break;
        }
    }

    // Return the handler
    return handler;

}

/**
 * @details Passes the event to the child that the mouse is over. (Event handling should be done by the derived class)
 *
 * @param x The x coordinate of the mouse
 * @param y The y coordinate of the mouse
 * @param button The button that was released
 */
void CompositeWidget::onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button) {

    // Loop through the children
    for(Vector<Widget*>::iterator childWidget = children.begin(); childWidget != children.end(); childWidget++){

        // Get the position of the child
        Rectangle<int> childArea = (*childWidget)->getPosition();

        // Check if the mouse is in the child
        if(childArea.contains(x, y)){

            // Get the position of the mouse relative to the child
            uint32_t childX = x - childArea.left;
            uint32_t childY = y - childArea.top;

            // Call the child's onMouseButtonReleased function
            (*childWidget)->onMouseButtonReleased(childX, childY, button);

            // Break as the event has been handled
            break;
        }
    }
}





