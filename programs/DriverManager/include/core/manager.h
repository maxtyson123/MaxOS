/**
 * @file manager.h
 * @brief Defines a Manager class for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_COMMON_MANAGER_H
#define DRIVER_MANAGER_COMMON_MANAGER_H

#include <driver.h>
#include <cstdint>
#include <cstddef>

#include <core/selector.h>

namespace DriverManager::core {


	/**
	 * @class Manager
	 * @brief Manages the drivers, handles the adding and removing of drivers
	 */
	class Manager : public SelectorEventHandler {

		private:
			MaxOS::common::Vector<LibDriver::Driver*> m_drivers;
			MaxOS::common::Vector<Selector*> m_driver_selectors;

		public:
			Manager();
			~Manager();

			void add_driver_selector(Selector*);
			void remove_driver_selector(Selector*);

			void add_driver(LibDriver::Driver*);
			void remove_driver(LibDriver::Driver*);
			void on_driver_selected(LibDriver::Driver*) final;

			void find_drivers();
			uint32_t reset_devices();
			void initialise_drivers();
			void deactivate_drivers();
			void activate_drivers();


	};

}

#endif //DRIVER_MANAGER_COMMON_MANAGER_H
