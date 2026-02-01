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

using namespace DriverManager;
using namespace DriverManager::core;
using namespace MaxOS::KPI::processes;

extern "C" void _start(void) {

	MaxOS::KPI::klog("DRIVER MANAGER STARTED\n");

	// Start the driver manager
	Manager driver_manager;
	driver_manager.find_drivers();

	// Reset the drivers
	uint32_t reset_wait_time = driver_manager.reset_devices();
	sleep(reset_wait_time);

	// Initialise the drivers
	driver_manager.initialise_drivers();
	driver_manager.activate_drivers();

	// Wait for events
	while (true) {
		asm("nop");
	}


}
