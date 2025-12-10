/**
 * @file mouse.cpp
 * @brief Implementation of a Mouse driver and event handler
 *
 * @date 10th October 2022
 * @author Max Tyson
 */

#include <generic/mouse.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace LibDriver::generic;


/**
 * @brief Constructs a new Mouse object and registers it as an interrupt handler for interrupt 0x2C
 */
Mouse::Mouse() = default;
Mouse::~Mouse() = default;

/**
 * @brief activate the mouse
 */
void Mouse::activate() {

}

/**
 * @brief Get the name of the device
 *
 * @return The name of the device
 */
string Mouse::device_name() {
	return "Mouse";
}