//
// Created by Max Tyson on 14/04/2026.
//

#include <libdriver/server/driver_server.h>
#include <libdriver/server/drivermanager_client.h>

#include <ide.h>

using namespace IDEDriver;
using namespace MaxOS::KPI;
using namespace MaxOS;
using namespace LibDriver;
using namespace LibDriver::HardwareCommunication;

extern "C" void _start(int argc, char* argv[])
{

	// Fetch the device id
	if (argc <= 1)
		return;
	string id = argv[1];

	// Initialise the device
	hardware_mapping_t hmap = get_hardware_mapping(id);
	IntegratedDriveElectronicsController ide(hmap);

	// Wait for events
	DriverServer server(&ide);
	driver_ready(id);
	server.start(id);
}