/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#include <cstdint>
#include <file.h>
#include <json.h>
#include <processes/thread.h>
#include <core/manager.h>
#include <core/pci.h>
#include <server/drivermanager_server.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace LibDriver::HardwareCommunication;
using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace LibFS;

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

extern "C" void _start(void) {

	// Start the driver manager
	driver_manager = new Manager();
	driver_manager -> find_devices();

	auto jhandle = open_file("/boot/initrd/initdrivers.json");
	auto size = file_size(jhandle);
	auto jsonstr = (char*)allocate_memory(size);
	file_read(jhandle, jsonstr, size);
	klog(jsonstr);
	klog("\n--\n");

	auto jstr = MaxOS::string(jsonstr);
	auto json = new JSONParser(&jstr);
	auto root = json->root();
	string test = (*root)["drivers"s][0]["name"s];
	klog(test.c_str());

	// Disks must be setup to load the found drivers
	driver_manager -> start_disks();

	// Start the servers
	uint64_t handle = register_drivermanager();
	while (true) {

		// Process events
		bool did_handle = rpc_server_process_next(handle, false);

		// Check if there is anything to do
		if (driver_manager -> all_drivers_started() && !did_handle) {
			yield();
			continue;
		}

		// Try start devices
		driver_manager -> start_drivers();
	}

	// Should never exit
	exit(-1);
}
