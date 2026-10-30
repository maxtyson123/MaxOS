/**
 * @file rawdatahandler.h
 * @brief Defines a RawDataHandler class for handling raw ethernet data received from the ethernet controller.
 *
 * @date 1st December 2022
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H
#define MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H

#include <cstdint>


namespace MaxOS::drivers::ethernet {

	/**
	 * @class RawDataHandler
	 * @brief Handles raw data received from the ethernet controller
	 */
	class RawDataHandler {
		public:
			RawDataHandler();
			~RawDataHandler();
			virtual bool handle_raw_data(uint8_t* data, uint32_t size);
	};


}


#endif //MAXOS_DRIVERS_ETHERNET_RAWDATAHANDLER_H
