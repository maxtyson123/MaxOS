/**
 * @file ide.h
 * @brief Defines a driver for the Integrated Drive Electronics (IDE) controller
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#ifndef IDEDRIVER_IDE_H
#define IDEDRIVER_IDE_H

#include <libdriver/device.h>
#include <libdriver/driver.h>
#include <libdriver/hardwarecommunication/mapping.h>
#include <libdriver/server/drivermanager_client.h>
#include <libcommon/port.h>

#include <constants.h>

namespace IDEDriver {

	/**
	* @class IntegratedDriveElectronicsController
	* @brief Driver for the IDE controller, handles the creation and management of the IDE devices
	*/
	class IntegratedDriveElectronicsController : public LibDriver::Driver {

		private:

			bool m_primary_is_legacy;
			bool m_secondary_is_legacy;

			bool identify_device(uint16_t command_base, bool master);
			void setup_ata(uint16_t command_base, uint16_t control_base, bool master);

		public:
			explicit IntegratedDriveElectronicsController(LibDriver::device_identification_t& device_id, LibDriver::HardwareCommunication::hardware_mapping_t& hmap);
			~IntegratedDriveElectronicsController() final;


	};
}


#endif //IDEDRIVER_IDE_H
