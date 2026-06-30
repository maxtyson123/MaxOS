/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#include <cstdint>

#include <libkpi/processes/thread.h>

#include <core/manager.h>
#include <core/pci.h>
#include <libdriver/server/drivermanager_server.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver::HardwareCommunication;
using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;

Manager* driver_manager = nullptr;


void driver_ready(mstring id) {

	if (!driver_manager)
		return;

	// Make sure the device exists
	auto device = driver_manager->get_device(id.to_int());
	if (!device)
		return;

	// Mark the driver as ready
	device->driver_started = true;

}


hardware_mapping_t get_hardware_mapping(mstring id) {

	if (!driver_manager)
		return {};

	// Make sure the device exists
	auto device = driver_manager->get_device(id.to_int());
	if (!device)
		return {};

	return device->hardware_mapping();
}

device_identification_t get_device_identification(mstring id) {


	if (!driver_manager)
		return {};

	// Make sure the device exists
	auto device = driver_manager->get_device(id.to_int());
	if (!device)
		return {};

	return device->id_info();
}

int register_device(hardware_mapping_t hmap, device_identification_t did) {

	// Register the device
	auto device = new Device(did, hmap);
	driver_manager->register_device(device);

	// Externally registered devices have to set up their own drivers
	device->driver_started = true;
	return  device->id;
}

extern "C" void _start(void) {

	// Start the driver manager
	driver_manager = new Manager();
	driver_manager -> find_devices();

	// Start the servers
	uint64_t handle = register_drivermanager();
	while (true) {

		// Process events
		bool did_work = rpc_server_process_next(handle, false);

		// Try start devices
		if (!driver_manager -> all_drivers_started()) {
			driver_manager -> start_drivers();
			did_work = true;
		}

		// If no work was done for either task then dont hog the cpu
		if (!did_work)
			yield();
	}

	// Should never exit
	exit(-1);
}
