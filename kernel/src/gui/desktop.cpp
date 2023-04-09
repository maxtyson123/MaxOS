//
// Created by 98max on 10/15/2022.
//

#include <gui/desktop.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::gui;

Desktop::Desktop(common::int32_t w, common::int32_t h, common::uint8_t r, common::uint8_t g, common::uint8_t b)
: CompositeWidget(0,0,0,w,h,r,g,b),
  MouseEventHandler()
{
    //Center Mouse
    MouseX = w/2;
    MouseY = h/2;
}

Desktop::~Desktop() {

}

/**
 * @details Draw the desktop onto a graphics context
 *
 * @param gc The graphics context to draw on
 */
void Desktop::Draw(common::GraphicsContext *gc) {

    //Draw the desktop
    CompositeWidget::Draw(gc);



    //Draw the mouse
    for(int i = 0; i < 4; i++)
    {
        //Make a cross shape
        gc -> PutPixel(MouseX-i, MouseY, 0xFF, 0xFF, 0xFF);
        gc -> PutPixel(MouseX+i, MouseY, 0xFF, 0xFF, 0xFF);
        gc -> PutPixel(MouseX, MouseY-i, 0xFF, 0xFF, 0xFF);
        gc -> PutPixel(MouseX, MouseY+i, 0xFF, 0xFF, 0xFF);
    }

}

/**
 * @details Handle mouse button release events
 *
 * @param button The mouse button that was released
 */
void Desktop::OnMouseUp(maxOS::common::uint8_t button) {

    //Translate mouseEvent to widget method
    CompositeWidget::OnMouseDown(MouseX,MouseY,button);
}

/**
 * @details Handle mouse button press events
 *
 * @param button The mouse button that was pressed
 */
void Desktop::OnMouseDown(maxOS::common::uint8_t button) {

    //Translate mouseEvent to widget method
    CompositeWidget::OnMouseDown(MouseX,MouseY,button);
};

/**
 * @details Handle mouse movement events
 *
 * @param x The new x position of the mouse
 * @param y The new y position of the mouse
 */
void Desktop::OnMouseMove(int x, int y) {

    //Divide Relative movement so that the mouse is not as sensitive
    x /= 4;
    y /= 4;

    //Apply change to mouse position
    int32_t newMouseX = MouseX + x;
    int32_t newMouseY = MouseY + y;

    //Make sure mouse position not out of bounds
    if(newMouseX < 0) x = 0;
    if(newMouseX >= w) x = w-1;

    if(newMouseY < 0) newMouseY = 0;
    if(newMouseY >= h) newMouseY = h-1;

    //Translate mouseEvent to widget method
    CompositeWidget::OnMouseMove(MouseX,MouseY,newMouseX,newMouseY);

    //Store new vals
    MouseX = newMouseX;
    MouseY = newMouseY;
}

