/**
 * @file keyboard.cpp
 * @brief Implementation of a Keyboard driver and event handling
 *
 * @date 10th September 2022
 * @author Max Tyson
 */

#include <libdriver/generic/keyboard.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace LibDriver;
using namespace LibDriver::generic;

/**
 * @brief Construct a new Keyboard Driver object
 */
Keyboard::Keyboard()
{


}

Keyboard::~Keyboard() = default;

KeyboardState::KeyboardState() = default;

KeyboardState::~KeyboardState() = default;

/**
 * @brief Construct a new Keyboard Interpreter object
 */
KeyboardInterpreter::KeyboardInterpreter()
: InputStreamEventHandler<uint8_t>()
{

}

KeyboardInterpreter::~KeyboardInterpreter() = default;

/**
 * @brief Handle the key event
 *
 * @param released True if the key was released, false if it was pressed
 * @param state The state of the keyboard
 * @param key_code The key code of the key
 */
void KeyboardInterpreter::on_key_read(bool released, const KeyboardState &state, KeyCode key_code) {

}

/**
 * @brief Construct a new Keyboard Interpreter for the EN_US layout
 */
KeyboardInterpreterEN_US::KeyboardInterpreterEN_US()
: KeyboardInterpreter()
{

}

KeyboardInterpreterEN_US::~KeyboardInterpreterEN_US() = default;

/**
 * @brief Handle the key down event
 *
 * @param scan_code The scan code of the key that was pressed
 *
 * @todo Probably a better way to do this (investigate when adding more keyboard layouts)
 */
void KeyboardInterpreterEN_US::on_stream_read(uint8_t scan_code) {

}