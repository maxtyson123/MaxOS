//
// Created by 98max on 10/15/2022.
//
#include <gui/window.h>
#include <gui/widgets/text.h>

using namespace maxOS;
using namespace maxOS::gui;
using namespace maxOS::common;

Window::Window(Widget *parent, common::int32_t x, common::int32_t y, common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b)
: CompositeWidget(parent, x, y, w, h, r, g, b) {
    Dragging = false;
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
void Window::OnMouseDown(common::int32_t x, common::int32_t y, common::uint8_t button) {
    //Check if clicking with left button
    Dragging = (button == 1);

    //Run default (So Children also receive event)
    CompositeWidget::OnMouseDown(x, y, button);
}

/**
 * @details This function is called when a mouse button is released.
 *
 * @param x The x coordinate of the mouse.
 * @param y The y coordinate of the mouse.
 * @param button The button that is released.
 */
void Window::OnMouseUp(common::int32_t x, common::int32_t y, common::uint8_t button) {

    Dragging = false;

    //Run default (So Children also receive event)
    CompositeWidget::OnMouseUp(x, y, button);
}

/**
 * @details This function is called when the mouse is moved.
 *
 * @param oldx The old x coordinate of the mouse.
 * @param oldy The old y coordinate of the mouse.
 * @param newx The new x coordinate of the mouse.
 * @param newy The new y coordinate of the mouse.
 */
void Window::OnMouseMove(common::int32_t old_x, common::int32_t old_y, common::int32_t new_x, common::int32_t new_y) {

    //Drag the window
    if(Dragging)
    {
        this->x += new_x-old_x;
        this->y += new_y-old_y;
    }

    //Run default (So Children also receive event)
    CompositeWidget::OnMouseMove(old_x, old_y, new_x, new_y);
}

/**
 * @details This function draws the window.
 *
 * @param gc The graphics context to draw on.
 */
void Window::Draw(common::GraphicsContext *gc) {


    //Set To 0 as ModelToScreen ads own x and y to it
    int X = 0;
    int Y = 0;

    //Get absolute position
    ModelToScreen(X,Y);

    //Draw Header
    gc->FillRectangle(X,Y-10,w,10,0xFF,0xFF,0xFF);

    //Draw window title


    CompositeWidget::Draw(gc);
}
