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

Window::Window(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height, common::string titleText)
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
    title.font.foregroundColour = Colour(0xff, 0xff, 0xff);    // White
    title.font.backgroundColour = windowFrameColour;

    // Add the title to the window
    Window::addChild(&title);

}

Window::Window(Widget *containedWidget, common::string titleText)
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
    title.font.foregroundColour = Colour(0xff, 0xff, 0xff);    // White
    title.font.backgroundColour = windowFrameColour;

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
MouseEventHandler* Window::onMouseButtonPressed(common::uint32_t x, common::uint32_t y, common::uint8_t button){

    // Get the position of the window
    Rectangle<int> windowPosition = this -> getPosition();

    // Check if the mouse is in the frame
    if(x <= windowFrameThickness){                  // This means the mouse is in the left as the x position (relative to this object) has not passed the frame

        // If y is also less than the frame thickness then the mouse is in the top left corner
        if(y <= windowFrameThickness){
            return &windowWidgetMoverResizerTopLeft;
        }

        // If y is also greater than the height of the window minus the frame thickness then the mouse is above the bottom left corner
        if(y >= windowPosition.height - windowFrameThickness){
            return &windowWidgetMoverResizerLeft;
        }

        // Otherwise it's in the bottom left corner
        return &windowWidgetMoverResizerBottomLeft;

    }else if (x < windowPosition.width - windowFrameThickness){    // This means the mouse is in the middle as the x position (relative to this object) has passed the frame on the left but not the right

        // If y is also less than the frame thickness then the mouse is in the top middle
        if(y <= windowFrameThickness){
            return &windowWidgetMoverResizerTop;
        }

        // If y less thant the frame thick ness and the height of the title bar then the mouse is in the title bar
        if(y <= windowFrameThickness + windowTitleBarHeight){
            return &windowWidgetMover;
        }

        // If y is also greater than the height of the window minus the frame thickness then the mouse is in the bottom
        if(y >= windowPosition.height - windowFrameThickness) {
            return &windowWidgetMoverResizerBottom;
        }

    }else{  // This means the only place left to check is the right as left and top/bottom is done (The mouse must be on the right otherwise it would have been picked up in the middle (prev if statement))

        // If y is also less than the frame thickness then the mouse is in the top right corner
        if (y <= windowFrameThickness) {
            return &windowWidgetMoverResizerTopRight;
        }

        // If y is also greater than the height of the window minus the frame thickness then the mouse is above the bottom right corner
        if (y >= windowPosition.height - windowFrameThickness) {
            return &windowWidgetMoverResizerRight;
        }

        // Otherwise its in the bottom right corner
        return &windowWidgetMoverResizerBottomRight;
    }

    // Pass the event on to the children and see which one handles it
    MouseEventHandler* childThatHandledEvent = CompositeWidget::onMouseButtonPressed(x, y, button);
    return childThatHandledEvent;

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

    // TODO: This is a hack to get the window border to draw correctly, should make it so that the window border is drawn in the correct place
    gc -> drawRectangle(windowX+1, windowY+1, windowX + windowPosition.width -1, windowY + windowPosition.height -1, windowFrameBorderColour);
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
