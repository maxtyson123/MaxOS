/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Driver Manager
 *
 * @date 24th November 2025
 * @author Max Tyson
 */

#include <cstdint>
#include <processes/thread.h>
#include <core/manager.h>
#include <core/pci.h>
#include <server/drivermanager_server.h>

using namespace DriverManager;
using namespace DriverManager::core;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;

extern "C" void _start(void) {

	// Start the driver manager
	Manager driver_manager;
	driver_manager.find_devices();

	// Disks must be setup to load the found drivers
	driver_manager.start_disks();

	// Start the servers
	uint64_t handle = register_drivermanager();

	while (true) {

		// Process events
		bool did_handle = rpc_server_process_next(handle, false);

		// Check if there is anything to do
		if (driver_manager.all_drivers_started() && !did_handle) {
			yield();
			continue;
		}

		// Try start devices
		driver_manager.start_drivers();
	}

	// Should never exit
	exit(-1);
}
