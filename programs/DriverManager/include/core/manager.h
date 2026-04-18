/**
 * @file manager.h
 * @brief Defines a Manager class for managing drivers
 *
 * @date 11th October 2022
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_CORE_MANAGER_H
#define DRIVER_MANAGER_CORE_MANAGER_H

#include <driver.h>
#include <cstdint>
#include <cstddef>

#include <core/device.h>

namespace DriverManager::core {


	/**
	 * @class Manager
	 * @brief Manages the drivers, handles the adding and removing of drivers
	 */
	class Manager : public DeviceEnumeratorEventHandler {

		private:
			size_t m_next_device_id = 0;
			MaxOS::common::Vector<Device*> m_devices;

			MaxOS::common::Vector<DeviceEnumerator*> m_device_enumerators;

		public:
			Manager();
			~Manager() final;

			void add_device_enumerator(DeviceEnumerator*);
			void remove_device_enumerator(DeviceEnumerator*);

			void find_devices();
			void register_device(Device* device);
			void on_device_enumerated(Device*) final;

			Device* get_device(int id);

			bool all_drivers_started();
			void start_drivers();
			void start_disks();

			size_t get_next_device_id();

	};

}

#endif //DRIVER_MANAGER_CORE_MANAGER_H
