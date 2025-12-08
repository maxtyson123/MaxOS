/**
 * @file keyboard.h
 * @brief Defines a Keyboard and related classes for handling keyboard input
 *
 * @date 9th October 2022
 * @author Max Tyson
 */

#ifndef MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H
#define MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H

#include <map.h>
#include <eventHandler.h>
#include <inputStream.h>
#include <string.h>
#include <vector.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/apic.h>
#include <cstdint>


namespace MaxOS::drivers::peripherals {

	/**
	 * @enum ScanCodeType
	 * @brief The type of scan code being sent by the keyboard
	 */
	enum class ScanCodeType : int {
		REGULAR,
		EXTENDED,
		EXTENDED_BUFFER
	};

	// PS2 Keyboard:
	// 	private:
	//			hardwarecommunication::Port8Bit m_data_port;
	//			hardwarecommunication::Port8Bit m_command_port;
	// & handle interrupts
}


#endif //MAX_OS_DRIVERS_PERIPHERALS_KEYBOARD_H