/**
 * @file ide.cpp
 * @brief Implementation of an Integrated Drive Electronics (IDE) controller driver
 *
 * @date 18th April 2025
 * @author Max Tyson
 */

#include <ide.h>


using namespace MaxOS;
using namespace MaxOS::common;
using namespace LibDriver;
using namespace IDEDriver;

/**
 * @brief Construct a new Integrated Drive Electronics Controller object
 *
 * @param device_descriptor The PCI device descriptor for this controller
 * @todo Use the device descriptor to get the port base and add the devices dynamically
 */
IntegratedDriveElectronicsController::IntegratedDriveElectronicsController(HardwareCommunication::hardware_mapping_t& hmap)
{

}

IntegratedDriveElectronicsController::~IntegratedDriveElectronicsController() = default;
//
// /**
//  * @brief Initialise the IDE controller by identifying the devices
//  */
// void IntegratedDriveElectronicsController::initialise() {
//
// 	// Loop through the devices and identify them
// 	for (auto &device: devices) {
//
// 		// Check if the device is present
// 		auto ata_device = device.first;
// 		if (ata_device == nullptr)
// 			continue;
//
// 		// Identify the device
// 		bool exists = ata_device->identify();
//
// 		// Remove the device if it does not exist
// 		if (!exists) {
// 			devices.erase(ata_device);
// 			delete ata_device;
// 			continue;
// 		}
// 	}
//
// 	// Log the init done
// 	Logger::DEBUG() << "IDE Controller: Initialised " << devices.size() << " devices\n";
// }
//
// /**
//  * @brief Activate the IDE controller by mounting the devices to the virtual file system
//  */
// void IntegratedDriveElectronicsController::activate() {
//
// 	// Loop through the devices and load the partitions
// 	for (auto &device: devices) {
//
// 		// Ensure there is a device and that it is the master
// 		if (device.first == nullptr || !device.second)
// 			continue;
//
// 		// Mount the device
// 		MSDOSPartition::mount_partitions(device.first);
//
// 	}
// }