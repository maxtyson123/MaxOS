/**
 * @file mouse.h
 * @brief Defines a MouseDriver for handling PS/2 mouse input and generating mouse events
 *
 * @date 10th October 2022
 * @author Max Tyson
 */

#ifndef MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
#define MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H


#include <cstdint>
#include <vector.h>
#include <string.h>
#include <eventHandler.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/driver.h>


namespace MaxOS::drivers::peripherals {

	/**
	 * @enum MouseEvents
	 * @brief The different types of mouse events that can be triggered
	 */
	enum class MouseEvents {
		MOVE,
		DOWN,
		UP
	};

	/**
	 * @class MouseMoveEvent
	 * @brief Event that is triggered when the mouse moves, holds the x and y coordinates
	 */
	class MouseMoveEvent : public common::Event<MouseEvents> {
		public:
			int8_t x;       ///< The x coordinate of the mouse
			int8_t y;       ///< The y coordinate of the mouse
			MouseMoveEvent(int8_t x, int8_t y);
			~MouseMoveEvent();
	};

	/**
	 * @class MouseDownEvent
	 * @brief Event that is triggered when a mouse button is pressed, holds the button that was pressed
	 */
	class MouseDownEvent : public common::Event<MouseEvents> {
		public:
			uint8_t button; ///< The button that was pressed
			explicit MouseDownEvent(uint8_t);
			~MouseDownEvent();
	};

	/**
	 * @class MouseUpEvent
	 * @brief Event that is triggered when a mouse button is released, holds the button that was released
	 */
	class MouseUpEvent : public common::Event<MouseEvents> {
		public:
			uint8_t button; ///< The button that was released
			explicit MouseUpEvent(uint8_t);
			~MouseUpEvent();
	};

	/**
	 * @class MouseEventHandler
	 * @brief Handles events that are triggered by the mouse driver
	 */
	class MouseEventHandler : public common::EventHandler<MouseEvents> {

		public:
			MouseEventHandler();
			~MouseEventHandler();

			common::Event<MouseEvents>*
			on_event(common::Event<MouseEvents>*) override;

			virtual void on_mouse_down_event(uint8_t button);
			virtual void on_mouse_up_event(uint8_t button);
			virtual void on_mouse_move_event(int8_t x, int8_t y);
	};

	/**
	 * @class MouseDriver
	 * @brief Driver for the PS/2 mouse, manages the mouse and triggers events when the mouse moves or a button is pressed
	 */
	class MouseDriver : public hardwarecommunication::InterruptHandler, public Driver, public common::EventManager<MouseEvents> {

		private:
			hardwarecommunication::Port8Bit data_port;
			hardwarecommunication::Port8Bit command_port;

			void handle_interrupt() final;

			uint8_t m_buffer[3] = { };
			uint8_t m_offset = 0;
			uint8_t m_buttons = 0;

		public:
			MouseDriver();
			~MouseDriver();

			void activate() final;
			string device_name() final;
	};
}


#endif //MAX_OS_DRIVERS_PERIPHERALS_MOUSE_H
