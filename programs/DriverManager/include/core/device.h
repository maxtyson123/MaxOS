/**
 * @file device.h
 * @brief Defines classes for enumerating and handling devices in the Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#ifndef DRIVER_MANAGER_COMMON_SELECTOR_H
#define DRIVER_MANAGER_COMMON_SELECTOR_H

#include <libdriver/driver.h>
#include <libdriver/device.h>
#include <libcommon/assert.h>


namespace DriverManager::core {

	typedef LibDriver::device_identification_t device_identification_t;

	/**
	 * @class Device
	 * @breif Generic representation of a physical device
	 *
	 * @note This is different from a driver, which is the software to interact with a device
	 */
	class Device {

		private:

			device_identification_t m_id_info;
			LibDriver::HardwareCommunication::hardware_mapping_t m_hmap;

		public:
			Device(device_identification_t id_info, LibDriver::HardwareCommunication::hardware_mapping_t hmap);
			virtual ~Device();

			device_identification_t id_info();
			LibDriver::HardwareCommunication::hardware_mapping_t hardware_mapping();

			size_t id = 0;
			bool driver_started = false;
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
