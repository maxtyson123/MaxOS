//
// Created by 98max on 10/10/2022.
//

#include <drivers/peripherals/mouse.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;
using namespace MaxOS::hardwarecommunication;


///__Handler__

MouseEventHandler::MouseEventHandler() = default;

/**
 * @brief Handles the triggered event and calls the appropriate function
 *
 * @param event The event that was triggered
 * @return The event that was triggered with the modified data
 */
Event<MouseEvents>* MouseEventHandler::on_event(Event<MouseEvents> *event) {
    switch (event->type){

        case MouseEvents::MOVE:
          this->on_mouse_move_event(((MouseMoveEvent *)event)->x,
                                    ((MouseMoveEvent *)event)->y);
            break;

        case MouseEvents::DOWN:
          this->on_mouse_down_event(((MouseDownEvent *)event)->button);
            break;

      case MouseEvents::UP:
          this->on_mouse_up_event(((MouseUpEvent *)event)->button);
            break;

    }

    // Return the event
    return event;
}

/**
 * @brief Called when the mouse is pressed
 *
 * @param button The button that was pressed
 */
void MouseEventHandler::on_mouse_down_event(uint8_t){

}

/**
 * @brief Called when the mouse is released
 *
 * @param button The button that was released
 */
void MouseEventHandler::on_mouse_up_event(uint8_t){

}

/**
 * @brief Called when the mouse is moved
 *
 * @param x How much the mouse moved in the x direction
 * @param y How much the mouse moved in the y direction
 */
void MouseEventHandler::on_mouse_move_event(int8_t, int8_t){

}

MouseEventHandler::~MouseEventHandler() = default;

///__Driver__

MouseDriver::MouseDriver()
: InterruptHandler(0x2C, 0xC, 0x28),
  data_port(0x60),
  command_port(0x64)
{

}
MouseDriver::~MouseDriver()= default;

/**
 * @brief activate the mouse
 */
void MouseDriver::activate() {



  //  Get the current state of the mouse
  command_port.write(0x20);
  uint8_t status = (data_port.read() | 2);

  // write the new state
  command_port.write(0x60);
  data_port.write(status);

  // Tell the PIC to start listening to the mouse
  command_port.write(0xAB);

  // activate the mouse
  command_port.write(0xD4);
  data_port.write(0xF4);
  data_port.read();
}

/**
 * @brief Handle the mouse interrupt
 */
void MouseDriver::handle_interrupt(){

    //Only if the 6th bit of data is one then there is data to handle
    uint8_t status = command_port.read();
    if(!(status & 0x20))
        return;

    // read the data and store it in the buffer
    buffer[offset] = data_port.read();
    offset = (offset + 1) % 3;

    // If the mouse data transmission is incomplete (3rd piece of data isn't through)
    if(offset != 0)
        return;

    // If the mouse is moved (y-axis is inverted)
    if(buffer[1] != 0 || buffer[2] != 0)
      raise_event(new MouseMoveEvent(buffer[1], -buffer[2]));

    for (int i = 0; i < 3; ++i) {

        // Check if the button state has changed
        if((buffer[0] & (0x1<<i)) != (buttons & (0x1<<i)))
        {
            // Check if the button is up or down
            if(buttons & (0x1<<i))
              raise_event(new MouseUpEvent(i + 1));
            else
              raise_event(new MouseDownEvent(i + 1));

        }
    }

    // Update the buttons
    buttons = buffer[0];
}

/**
 * @brief Get the name of the device
 *
 * @return The name of the device
 */
string MouseDriver::device_name() {
    return "Mouse";
}

///__Events__

MouseUpEvent::MouseUpEvent(uint8_t button)
: Event<MouseEvents>(MouseEvents::UP),
  button(button)
{

}

MouseUpEvent::~MouseUpEvent() = default;

MouseDownEvent::MouseDownEvent(uint8_t button)
: Event<MouseEvents>(MouseEvents::DOWN),
  button(button)
{
}

MouseDownEvent::~MouseDownEvent() = default;

MouseMoveEvent::MouseMoveEvent(int8_t x, int8_t y)
: Event<MouseEvents>(MouseEvents::MOVE),
  x(x),
  y(y)
{
}

MouseMoveEvent::~MouseMoveEvent() = default;
