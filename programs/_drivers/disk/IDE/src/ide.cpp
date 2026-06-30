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
using namespace LibDriver::HardwareCommunication;
using namespace IDEDriver;
using namespace KPI;
using namespace KPI::processes;

/**
 * @brief Construct a new Integrated Drive Electronics Controller object
 *
 */
IntegratedDriveElectronicsController::IntegratedDriveElectronicsController(device_identification_t& device_id, hardware_mapping_t& hmap)
{

	// Detect the mode of the IDE
	m_primary_is_legacy = ((device_id.class_info.interface & 0x01) == 0);
	m_secondary_is_legacy = ((device_id.class_info.interface & 0x04) == 0);
	klog("IDE running in %s/%s mode\n", m_primary_is_legacy ? "legacy" : "native", m_secondary_is_legacy ? "legacy" : "native");

	// Set up the ports
	uint16_t primary_command_port = m_primary_is_legacy  ? LEGACY_PRIMARY_COMMAND_BASE : hmap.ranges[0].address_base;
	uint16_t primary_control_port = m_primary_is_legacy  ? LEGACY_PRIMARY_CONTROL_BASE : hmap.ranges[1].address_base;
	uint16_t secondary_command_port = m_secondary_is_legacy  ? LEGACY_SECONDARY_COMMAND_BASE : hmap.ranges[2].address_base;
	uint16_t secondary_control_port = m_secondary_is_legacy  ? LEGACY_SECONDARY_CONTROL_BASE : hmap.ranges[3].address_base;

	// Identify the ATA devices
	setup_ata(primary_command_port, primary_control_port, true);
	setup_ata(primary_command_port, primary_control_port, false);
	setup_ata(secondary_command_port, secondary_control_port, true);
	setup_ata(secondary_command_port, secondary_control_port, false);

}


IntegratedDriveElectronicsController::~IntegratedDriveElectronicsController() = default;

bool IntegratedDriveElectronicsController::identify_device(uint16_t command_base, bool master) {

	// Get the ports
	auto data_port = Port16Bit(command_base);
	auto device_port = Port8Bit(command_base  + 6);
	auto command_port = Port8Bit(command_base + 7);

	// Check if the master is present
	device_port.write(0xA0);
	uint8_t status = command_port.read();
	if (status == 0xFF)
		return false;

	// Select the device (master or slave)
	device_port.write(master ? 0xA0 : 0xB0);

	// Check if the device is present
	command_port.write(0x0EC);
	status = command_port.read();
	if (status == 0x00)
		return false;

	// Wait for the device to be ready or for an error to occur
	while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
		status = command_port.read();

	//Check for any errors
	if (status & 0x01)
		return false;

	// Skip until the start of the name
	uint16_t buffer[256];
	for (int i = 0; i < 256; i++)
		buffer[i] = data_port.read();

	// Get Name of the disk
	string name = " "s * 40;
	for (int i = 0; i < 20; i++) {
		name[i * 2]     = (buffer[27 + i] >> 8) & 0xFF;
		name[i * 2 + 1] = buffer[27 + i] & 0xFF;
	}
	name = name.strip();

	// Device is present and ready
	klog("Found %s ata on port 0x%x (%s)\n", master ? "master" : "slave", command_base, name.c_str());
	return true;
}

void IntegratedDriveElectronicsController::setup_ata(uint16_t command_base, uint16_t control_base, bool master) {

	if (!identify_device(command_base, master))
		return;

	// Store device info
	device_identification_t device_identification {};
	device_identification.driver_type = DriverType::DISK;
	device_identification.class_info.interface = master;

	hardware_mapping_t hardware_mapping{};
	hardware_mapping.ranges[0] = {
		.address_base = command_base,
		.address_length = 1,
		.type = HardwareRangeType::PORT
	};
	hardware_mapping.ranges[1] = {
		.address_base = control_base,
		.address_length = 1,
		.type = HardwareRangeType::PORT
	};

	// Register device
	int id = register_device(hardware_mapping, device_identification);

	// Start driver (@todo assumes ata is also in intrid)
	const char** args =  new const char*[1];
	args[0] = string(id).c_str();
	exec_file("ATADriver", "/0/boot/initrd/ATADriver.elf", args, 1);
}


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