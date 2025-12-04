/**
 * @file ide.h
 * @brief Defines a driver for the Integrated Drive Electronics (IDE) controller
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_IDE_H
#define MAXOS_DRIVERS_IDE_H

#include <drivers/disk/ata.h>
#include <cstdint>
#include <map.h>
#include <hardwarecommunication/pci.h>
#include <filesystem/partition/msdos.h>


namespace MaxOS::drivers::disk {

	/**
	* @class IntegratedDriveElectronicsController
	* @brief Driver for the IDE controller, handles the creation and management of the IDE devices
	*/
	class IntegratedDriveElectronicsController : public Driver {
		private:
			common::Map<AdvancedTechnologyAttachment*, bool> devices;

		public:
			explicit IntegratedDriveElectronicsController(hardwarecommunication::PCIDeviceDescriptor* device_descriptor);
			~IntegratedDriveElectronicsController();

			void initialise() final;
			void activate() final;

			string vendor_name() final;
			string device_name() final;


	};
}


#endif //MAXOS_DRIVERS_IDE_H
