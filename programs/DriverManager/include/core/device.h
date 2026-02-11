/**
 * @file device.h
 * @brief Defines classes for enumerating and handling devices in the Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_COMMON_SELECTOR_H
#define DRIVER_MANAGER_COMMON_SELECTOR_H

#include <driver.h>

namespace DriverManager::core {

	/**
	 * @class Device
	 * @breif Generic representation of a physical device
	 *
	 * @note This is different from a driver, which is the software to interact with a device
	 */
	class Device {

		private:

			bool m_driver_started = false;
			LibDriver::Driver* m_driver = nullptr;
			LibDriver::DriverType m_driver_type;

			virtual LibDriver::Driver* handle_driver_start();

		public:
			Device(LibDriver::DriverType driver_type);
			virtual ~Device();

			bool driver_started();
			LibDriver::DriverType driver_type();
			LibDriver::Driver* driver();

			size_t id = 0;

			void start_driver();


	};

	/**
	 * @class DeviceEnumeratorEventHandler
	 * @brief Event handler for the Selector class, handles the event when a driver is selected
	 */
	class DeviceEnumeratorEventHandler {
		public:
			DeviceEnumeratorEventHandler();
			virtual ~DeviceEnumeratorEventHandler();

			virtual void on_device_enumerated(Device*);
	};

	/**
	 * @class DeviceEnumerator
	 * @brief Enumerates a hub of devices
	 */
	class DeviceEnumerator {

		public:
			DeviceEnumerator();
			virtual ~DeviceEnumerator();

			virtual void enumerate_devices(DeviceEnumeratorEventHandler* handler);
	};

}

#endif //DRIVER_MANAGER_COMMON_SELECTOR_H
