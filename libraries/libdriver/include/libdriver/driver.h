/**
 * @file driver.h
 * @brief Defines a base Driver class that all drivers should inherit from
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#ifndef LIBDRIVER_DRIVER_H
#define LIBDRIVER_DRIVER_H

#include <cstdint>

#include <libkpi/ipc/rpc.h>
#include <libcommon/string.h>
#include <libdriver/hardwarecommunication/mapping.h>

namespace LibDriver {

	enum class  DriverType {
		UNKNOWN,
		DISK,
		ETHERNET,
		KEYBOARD,
		MOUSE,
		VIDEO,
	};


	/**
	 * @class Driver
	 * @brief base class for all drivers, handles the activation, deactivation, initialisation and reset of the driver as well as error messages and identifying the device
	 *
	 * @note This is different from a device, which is the physical hardware that a driver talks to
	 */
	class Driver {
		public:

			Driver();
			virtual ~Driver();

			virtual uint32_t reset();
	};
}

#endif //LIBDRIVER_DRIVER_H
