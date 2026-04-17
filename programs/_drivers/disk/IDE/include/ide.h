/**
 * @file ide.h
 * @brief Defines a driver for the Integrated Drive Electronics (IDE) controller
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#ifndef IDEDRIVER_IDE_H
#define IDEDRIVER_IDE_H

#include <libdriver/include/driver.h>
#include <libdriver/include/hardwarecommunication/mapping.h>

namespace IDEDriver {

	/**
	* @class IntegratedDriveElectronicsController
	* @brief Driver for the IDE controller, handles the creation and management of the IDE devices
	*/
	class IntegratedDriveElectronicsController : public LibDriver::Driver {

		public:
			explicit IntegratedDriveElectronicsController(LibDriver::HardwareCommunication::hardware_mapping_t& hmap);
			~IntegratedDriveElectronicsController() final;

			void initialise() final;
			void activate() final;

			MaxOS::string vendor_name() final;
			MaxOS::string device_name() final;


	};
}


#endif //IDEDRIVER_IDE_H
