/**
 * @file mouse.h
 * @brief Defines a Mouse for handling PS/2 mouse input and generating mouse events
 *
 * @date 10th October 2022
 * @author Max Tyson
 */

#ifndef LIBDRIVER_GENERIC__MOUSE_H
#define LIBDRIVER_GENERIC__MOUSE_H


#include <cstdint>
#include <driver.h>


namespace LibDriver::generic {

	/**
	 * @class Mouse
	 * @brief Driver for the PS/2 mouse, manages the mouse and triggers events when the mouse moves or a button is pressed
	 */
	class Mouse : public Driver {

		public:
			Mouse();
			~Mouse();

	};
}


#endif //LIBDRIVER_GENERIC__MOUSE_H