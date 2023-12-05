//
// Created by 98max on 10/15/2022.
//
#include <gui/window.h>
#include <gui/widgets/text.h>

using namespace maxOS;
using namespace maxOS::gui;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;

Window::Window(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height, string titleText)
: CompositeWidget(left, top, width, height),
// Top at -10 for bar height, -5 for frame, + 2 for the border. Width is the width of the window minus the frame on each side. Height is the height of the title bar plus the frame on the top minus the border.
  title(0, -(10 + 5) + 2, width - 2 * 5, 10 + 5 - 3, titleText),
  windowWidgetMover(this),
  windowWidgetMoverResizerTop(this),
  windowWidgetMoverResizerBottom(this),
  windowWidgetMoverResizerLeft(this),
  windowWidgetMoverResizerRight(this),
  windowWidgetMoverResizerTopLeft(this),
  windowWidgetMoverResizerTopRight(this),
  windowWidgetMoverResizerBottomLeft(this),
  windowWidgetMoverResizerBottomRight(this)
{
    // Set up the window border
    windowFrameThickness = 5;
    windowTitleBarHeight = 10;

    // Set the sizing
    minWidth = 2 * windowFrameThickness;
    minHeight = 2 * windowFrameThickness + windowTitleBarHeight;

    // Set the colours
    windowAreaColour            = Colour(0xff, 0xff, 0xff);    // White
    windowFrameBorderColour     = Colour(0x00, 0x00, 0x00);    // Black
    windowFrameColour           = Colour(0x57,0x57,0x57);      // Davy's Grey
    title.foregroundColour = Colour(0xff, 0xff, 0xff);    // White
    title.backgroundColour = windowFrameColour;

    // Add the title to the window
    Window::addChild(&title);

}

Window::Window(Widget *containedWidget, string titleText)
// Width is the width of the contained widget plus the frame on each side and the border. Height is the height of the contained widget plus the frame on the top and bottom plus the height of the title bar plus the border.
: CompositeWidget(0, 0, containedWidget -> getPosition().width + 2 * 5 + 2, containedWidget->getPosition().height + 2 * 5 + 10 + 2),
  title(0, -(10 + 5) + 2, containedWidget -> getPosition().width, 10 + 5 - 3, titleText),
  windowWidgetMover(this),
  windowWidgetMoverResizerTop(this),
  windowWidgetMoverResizerLeft(this),
  windowWidgetMoverResizerRight(this),
  windowWidgetMoverResizerBottom(this),
  windowWidgetMoverResizerTopLeft(this),
  windowWidgetMoverResizerTopRight(this),
  windowWidgetMoverResizerBottomLeft(this),
  windowWidgetMoverResizerBottomRight(this)
{
    // Set up the window border
    windowFrameThickness = 5;
    windowTitleBarHeight = 10;

    // Set the sizing
    minWidth = 2 * windowFrameThickness;
    minHeight = 2 * windowFrameThickness + windowTitleBarHeight;

    // Set the colours
    windowAreaColour            = Colour(0xff, 0xff, 0xff);    // White
    windowFrameBorderColour     = Colour(0x00, 0x00, 0x00);    // Black
    windowFrameColour           = Colour(0x57,0x57,0x57);      // Davy's Grey
    title.foregroundColour = Colour(0xff, 0xff, 0xff);    // White
    title.backgroundColour = windowFrameColour;

    // Add the title to the window
    Window::addChild(&title);
    Window::addChild(containedWidget);

}

Window::~Window() {

}

/**
 * @details This function is called when a button is pressed.
 *
 * @param x The x coordinate of the mouse.
 * @param y The y coordinate of the mouse.
 * @param button The button that is pressed.
 */
MouseEventHandler* Window::onMouseButtonPressed(common::uint32_t mouseX, common::uint32_t mouseY, common::uint8_t button){

    maxOS::drivers::peripherals::MouseEventHandler* childrenResult = CompositeWidget::onMouseButtonPressed(mouseX,mouseY,button);
    Rectangle<int> windowPosition = getPosition();

    // Check if the mouse is in the frame
    if(mouseX <= windowFrameThickness)                                                  // This means the mouse is in the left as the x position (relative to this object) has not passed the frame
    {
        if(mouseY <= windowFrameThickness)                                              // If y is also less than the frame thickness then the mouse is in the top left corner
            return &windowWidgetMoverResizerTopLeft;

        else if(mouseY < windowPosition.height - windowFrameThickness)                  // If y is also greater than the height of the window minus the frame thickness then the mouse is above the bottom left corner
            return &windowWidgetMoverResizerLeft;

        else                                                                            // Otherwise it's in the bottom left corner
            return &windowWidgetMoverResizerBottomLeft;
    }
    else if(mouseX < windowPosition.width - windowFrameThickness)                       // This means the mouse is in the middle as the x position (relative to this object) has passed the frame on the left but not the right
    {
        if(mouseY <= windowFrameThickness)                                              // If y is also less than the frame thickness then the mouse is in the top middle
            return &windowWidgetMoverResizerTop;

        else if(mouseY < windowFrameThickness+windowTitleBarHeight)                     // If y less thant the frame thickness and the height of the title bar then the mouse is in the title bar
            return &windowWidgetMover;

        else if(mouseY >= windowPosition.height-windowFrameThickness)                   // If y is also greater than the height of the window minus the frame thickness then the mouse is in the bottom
            return &windowWidgetMoverResizerBottom;
    }
    else
    {
        if(mouseY <= windowFrameThickness)                                              // If y is also less than the frame thickness then the mouse is in the top right corner
            return &windowWidgetMoverResizerTopRight;

        else if(mouseY < windowPosition.height-windowFrameThickness)                    // If y is also greater than the height of the window minus the frame thickness then the mouse is above the bottom right corner
            return &windowWidgetMoverResizerRight;

        else                                                                            // Otherwise its in the bottom right corner
            return &windowWidgetMoverResizerBottomRight;
    }


    return childrenResult;

}

/**
 * @details This function draws the window.
 *
 * @param gc The graphics context to draw on.
 */
void Window::drawSelf(common::GraphicsContext* gc, common::Rectangle<int>& area){

    // Get the positioning of the window
    Coordinates windowAbsolutePosition = CompositeWidget::absoluteCoordinates(Coordinates(0,0));
    Rectangle<int> windowPosition = this -> getPosition();
    int32_t windowX = windowAbsolutePosition.first;
    int32_t windowY = windowAbsolutePosition.second;

    // Create an area for the window contents
    Rectangle<int> windowContentsArea(windowFrameThickness, windowFrameThickness + windowTitleBarHeight, windowPosition.width - 2 * windowFrameThickness, windowPosition.height - 2 * windowFrameThickness - windowTitleBarHeight);

    // Check if the window contents area is in the area to draw
    if(windowContentsArea.intersects(area)){

        // Get the parts of the window contents area that are in the area to draw
        Rectangle<int> windowContentsAreaToDraw = windowContentsArea.intersection(area);

        // Draw the window contents area (adding windowX and windowY to draw in the correct place)
        gc -> fillRectangle(windowContentsAreaToDraw.left + windowX, windowContentsAreaToDraw.top + windowY, windowContentsAreaToDraw.left + windowContentsAreaToDraw.width + windowX, windowContentsAreaToDraw.top + windowContentsAreaToDraw.height + windowY, windowAreaColour);
    }

    // Draw the top of the window frame and the title bar (does not include left and right borders so start at windowFrameThickness and end at windowPosition.width - windowFrameThickness)
    Rectangle<int> windowFrameTopArea(windowFrameThickness, 0, windowPosition.width - 2 * windowFrameThickness, windowFrameThickness + windowTitleBarHeight);
    if(windowFrameTopArea.intersects(area)){

        // Get the parts of the window frame top area that are in the area to draw
        Rectangle<int> windowFrameTopAreaToDraw = windowFrameTopArea.intersection(area);

        // Draw the window frame top area (adding windowX and windowY to draw in the correct place)
        gc -> fillRectangle(windowFrameTopAreaToDraw.left + windowX, windowFrameTopAreaToDraw.top + windowY, windowFrameTopAreaToDraw.left + windowFrameTopAreaToDraw.width + windowX, windowFrameTopAreaToDraw.top + windowFrameTopAreaToDraw.height + windowY, windowFrameColour);

    }

    // Draw the bottom of the window frame (does not include left and right borders so start at windowFrameThickness and end at windowPosition.width - windowFrameThickness)
    Rectangle<int> windowFrameBottomArea(windowFrameThickness, windowPosition.height - windowFrameThickness, windowPosition.width - 2*windowFrameThickness, windowFrameThickness);
    if(windowFrameBottomArea.intersects(area)){

        // Get the parts of the window frame top area that are in the area to draw
        Rectangle<int> windowFrameBottomAreaToDraw = windowFrameBottomArea.intersection(area);

        // Draw the window frame top area (adding windowX and windowY to draw in the correct place)
        gc -> fillRectangle(windowX + windowFrameBottomAreaToDraw.left, windowY + windowFrameBottomAreaToDraw.top, windowX + windowFrameBottomAreaToDraw.left + windowFrameBottomAreaToDraw.width, windowY + windowFrameBottomAreaToDraw.top + windowFrameBottomAreaToDraw.height, windowFrameColour);

    }

    // Draw the left of the window frame
    Rectangle<int> windowFrameLeftArea(0,0, windowFrameThickness, windowPosition.height);
    if(windowFrameLeftArea.intersects(area)){

        // Get the parts of the window frame top area that are in the area to draw
        Rectangle<int> windowFrameLeftAreaToDraw = windowFrameLeftArea.intersection(area);

        // Draw the window frame top area (adding windowX and windowY to draw in the correct place)
        gc -> fillRectangle(windowX + windowFrameLeftAreaToDraw.left, windowY + windowFrameLeftAreaToDraw.top, windowX + windowFrameLeftAreaToDraw.left + windowFrameLeftAreaToDraw.width, windowY + windowFrameLeftAreaToDraw.top + windowFrameLeftAreaToDraw.height, windowFrameColour);

    }

    // Draw the right of the window frame
    Rectangle<int> windowFrameRightArea(windowPosition.width - windowFrameThickness, 0, windowFrameThickness, windowPosition.height);
    if(windowFrameRightArea.intersects(area)){

        // Get the parts of the window frame top area that are in the area to draw
        Rectangle<int> windowFrameRightAreaToDraw = windowFrameRightArea.intersection(area);

        // Draw the window frame top area (adding windowX and windowY to draw in the correct place)
        gc -> fillRectangle(windowX + windowFrameRightAreaToDraw.left, windowY + windowFrameRightAreaToDraw.top, windowX + windowFrameRightAreaToDraw.left + windowFrameRightAreaToDraw.width, windowY + windowFrameRightAreaToDraw.top + windowFrameRightAreaToDraw.height, windowFrameColour);

    }
}

/**
 * @details Adds a child to the window.
 *
 * @param child The child to add.
 */

void Window::addChild(Widget *child) {

    // If there is a child to add
    if(child != 0){

        // Get the position of the child
        Rectangle<int> childPosition = child -> getPosition();

        // Set the position of the child to be relative to the window frame and border
        child -> move(childPosition.left + windowFrameThickness + 1, childPosition.top + windowFrameThickness + windowTitleBarHeight + 1);

    }

    // Add the child to the window
    CompositeWidget::addChild(child);
}
