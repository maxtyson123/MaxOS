//
// Created by 98max on 10/15/2022.
//

#include <gui/desktop.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::drivers::peripherals;

/**
 * @details Creates a new desktop with the given graphics context
 *
 * @param gc The graphics context to use
 */
Desktop::Desktop(GraphicsContext *gc)
: CompositeWidget(0,0,gc -> getWidth(), gc -> getHeight()),
  MouseEventHandler(),
  ClockEventHandler()
{

    // Dragged and focussed widget are null
    draggedWidget = 0;
    focussedWidget = 0;

    // Set the graphics context
    graphicsContext = gc;

    // Set the mouse position to the center of the screen
    mouseX = gc -> getWidth() / 2;
    mouseY = gc -> getHeight() / 2;

    // Set the desktop colour
    colour = Colour(0xA8, 0xA8, 0xA8);    // Dark Gray (X11)

    // Draw the initial mouse cursor
    invertMouseCursor();

    // Draw the desktop
    Widget::invalidate();
}

Desktop::~Desktop() {

}

/**
 * @details Updates the currently focussed widget to be the given widget
 *
 * @param widget The widget that is now in focus
 */
void Desktop::setFocus(Widget* widget) {

    // Check if there already was a focussed widget
    if(this -> focussedWidget != 0){

        // Send a focus lost event to the old focussed widget
        this -> focussedWidget -> onFocusLost();

    }

    // Set the new focussed widget
    this -> focussedWidget = widget;

    // Send a focus gained event to the new focussed widget
    this -> focussedWidget -> onFocus();

}

/**
 * @details Brings the given widget to the front of the desktop
 *
 * @param frontWidget The widget to bring to the front
 */
void Desktop::bringToFront(Widget* frontWidget) {

    // Remove the widget from where ever it already is
    children.erase(frontWidget);

    // Add it back in the front
    children.pushFront(frontWidget);

}

/**
 * @details Draws the mouse cursor at the current mouse position by inverting the pixels (mouse is a plus sign)
 */
void Desktop::invertMouseCursor() {

    //TODO: Get image drawing going and draw a proper mouse

    // Draw the horizontal line
    for (int x = mouseX - 3; x <= mouseX + 3; ++x) {
        graphicsContext->invertPixel(x, mouseY);
    }

    // Draw the vertical line
    for (int y = mouseY - 3; y <= mouseY + 3; ++y) {
        graphicsContext->invertPixel(mouseX, y);
    }
}

/**
 * @details Goes through the passed areas and invalidates the areas that are covered by the given area
 *
 * @param area The area that is now invalid
 * @param start The start of the invalid areas
 * @param stop The end of the invalid areas
 */
void Desktop::internalInvalidate(Rectangle<int32_t> &area, Vector<Rectangle<int32_t>>::iterator start, Vector<Rectangle<int32_t>>::iterator stop) {

    // Loop through the invalid rectangles
    for(Vector<Rectangle<int32_t>>::iterator invaildRect = start; invaildRect != stop; invaildRect++){

        // Check if the area intersects with the invalid rectangle
        if(!area.intersects(*invaildRect))
            continue;

        // Get the parts of the area that are covered by the invalid rectangle
        Vector<Rectangle<int32_t>> coveredAreas = area.subtract(*invaildRect);

        // Loop through the covered areas
        for(Vector<Rectangle<int32_t>>::iterator coveredArea = coveredAreas.begin(); coveredArea != coveredAreas.end(); coveredArea++){

            // Invalidate the covered area
            internalInvalidate(*coveredArea, invaildRect + 1, stop);

        }

        // The entire area will be invalidated by now
        return;

    }

    // Add the area to the invalid areas, store where it was added
    Vector<Rectangle<int> >::iterator vectorPosition = invalidAreas.pushBack(area);

    // If the position is the last item then the invalidation buffer is full
    if(vectorPosition == invalidAreas.end()){

        // Clear the invalid areas
        invalidAreas.clear();

        // Invalidate the whole desktop
        Widget::invalidate();

    }
}

/**
 * @details Draws a certain area of the desktop
 *
 * @param gc The graphics context to draw with
 * @param area The area to draw
 */
void Desktop::drawSelf(common::GraphicsContext *gc, Rectangle<common::int32_t> &area) {

    //TODO: Draw a background image instead

    // Calculate the rectangle
    int32_t topCornerX = area.left;
    int32_t topCornerY = area.top;

    int32_t bottomCornerX = area.left + area.width;
    int32_t bottomCornerY = area.top + area.height;

    // Draw the background, a rectangle the size of the desktop of the given colour
    gc -> fillRectangle(topCornerX, topCornerY, bottomCornerX, bottomCornerY, colour);

}

/**
 * @details Adds a child widget to the desktop
 *
 * @param widget The widget to add
 */
void Desktop::addChild(Widget* childWidget) {

    // Check if the new widget is under the mouse
    bool underMouse = childWidget -> containsCoordinate(mouseX, mouseY);

    // If the mouse is over the new widget then send a mouse leave event to anything that used to be in this position
    if(underMouse){

        // Handle the mouse leaving this spot (as it has entered the new widget)
        CompositeWidget::onMouseLeaveWidget(mouseX, mouseY);
    }

    // Add the widget to the desktop
    CompositeWidget::addChild(childWidget);

    // If the mouse is over the new widget then send a mouse enter event to the child widget
    if(underMouse){
        CompositeWidget::onMouseEnterWidget(mouseX, mouseY);
    }
}

/**
 * @details Redraws the desktop when a time event occurs
 *
 * @param time The time when the event occurred
 */
void Desktop::onTime(const Time &time) {

    // Check if anything is invaild and needs to be redrawn
    if(invalidAreas.empty())
        return;

    // Erase the mouse cursor
    invertMouseCursor();

    // Loop through the invalid areas
    while (!invalidAreas.empty()) {

        // Get the first invalid area
        Rectangle<int32_t> invalidArea = *(invalidAreas.begin());   //get the pointer to the first element

        // Remove the area from the invalid areas
        invalidAreas.popFront();

        // Draw the area
        draw(graphicsContext, invalidArea);

    }

    // Can now draw the mouse cursor
    invertMouseCursor();

}

/**
 * @details Invalidate the given area of the desktop
 *
 * @param area The area that is now invalid
 */
void Desktop::invalidate(Rectangle<common::int32_t> &area) {

    // Invalidate the area
    internalInvalidate(area, invalidAreas.begin(), invalidAreas.end());

}


/**
 * @details When the mouse moves on the desktop update the position of the mouse and redraw the cursor. Pass the event to the widget that the mouse is over
 *
 * @param x The x position of the mouse
 * @param y The y position of the mouse
 */
void Desktop::onMouseMoveEvent(int8_t x, int8_t y) {

    // Store the position of the desktop for calculations
    Rectangle<int> desktopPosition = getPosition();

    // Calculate the position of the mouse on the desktop
    int32_t newMouseX = mouseX + x;
    int32_t newMouseY = mouseY + y;

    // Restrain the mouse to the desktop
    if(newMouseX < 0) newMouseX = 0;
    if(newMouseY < 0) newMouseY = 0;
    if(newMouseX > desktopPosition.width) newMouseX = desktopPosition.width - 1;
    if(newMouseY > desktopPosition.height) newMouseY = desktopPosition.height - 1;

    // Remove the old cursor from the screen as it will be redrawn in the new position
    invertMouseCursor();

    // Check if the mouse is currently dragging a widget
    if(draggedWidget != 0)
    {
        // Pass the mouse move event to the widget being dragged
        draggedWidget -> onMouseMoveEvent(newMouseX - mouseX, newMouseY - mouseY);
    }

    // Handle the mouse moving event (pass it to the widget that the mouse is over)
    CompositeWidget::onMouseMoveWidget(mouseX, mouseY, newMouseX, newMouseY);

    // Update the mouse position
    mouseX = newMouseX;
    mouseY = newMouseY;

    // Draw the new cursor
    invertMouseCursor();
}

/**
 * @details When the mouse button is pressed pass the event to the widget that the mouse is over
 *
 * @param button The button that was pressed
 */
void Desktop::onMouseDownEvent(uint8_t button) {

    // The widget that handled the event becomes the widget being dragged
    draggedWidget = CompositeWidget::onMouseButtonPressed(mouseX, mouseY, button);

}

/**
 * @details When the mouse button is released pass the event to the widget that the mouse is over
 *
 * @param button The button that was released
 */
void Desktop::onMouseUpEvent(uint8_t button) {

    // Pass the event to the widget
    CompositeWidget::onMouseButtonReleased(mouseX, mouseY, button);

    // Dragging has stopped
    draggedWidget = 0;

}

/**
 * @details When a key is pressed pass the event to the widget that is currently focussed
 * @param keyDownCode The key that was pressed
 * @param keyDownState The state of the keyboard
 */
void Desktop::onKeyDown(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState) {

    // Pass the event to the widget that is  in focus
    if (focussedWidget != 0) {
        focussedWidget -> onKeyDown(keyDownCode, keyDownState);
    }
}

/**
 * @details When a key is pressed pass the event to the widget that is currently focussed
 * @param keyUpCode The key that was pressed
 * @param keyUpState The state of the keyboard
 */
void Desktop::onKeyUp(drivers::peripherals::KeyCode keyUpCode, drivers::peripherals::KeyboardState keyUpState) {

    // Pass the event to the widget that is  in focus
    if (focussedWidget != 0) {
        focussedWidget -> onKeyUp(keyUpCode, keyUpState);
    }
}

