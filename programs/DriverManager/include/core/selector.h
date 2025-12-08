/**
 * @file selector.h
 * @brief Defines classes for selecting drivers in the Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_COMMON_SELECTOR_H
#define DRIVER_MANAGER_COMMON_SELECTOR_H

#include <driver.h>

namespace DriverManager::core {

	/**
	 * @class SelectorEventHandler
	 * @brief Event handler for the Selector class, handles the event when a driver is selected
	 */
	class SelectorEventHandler {
		public:
			SelectorEventHandler();
			~SelectorEventHandler();
			virtual void on_driver_selected(LibDriver::Driver*);
	};

	/**
	 * @class Selector
	 * @brief Selects the drivers to be used
	 */
	class Selector {
		public:
			Selector();
			virtual ~Selector();
			virtual void select_drivers(SelectorEventHandler* handler);
	};

}

#endif //DRIVER_MANAGER_COMMON_SELECTOR_H
