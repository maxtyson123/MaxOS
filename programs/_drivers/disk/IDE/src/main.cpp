//
// Created by Max Tyson on 14/04/2026.
//

#include <ide.h>
#include <server/driver_server.h>
#include <server/drivermanager_client.h>

using namespace IDEDriver;
using namespace MaxOS;
using namespace LibDriver;
using namespace LibDriver::HardwareCommunication;

extern "C" void _start(int argc, char* argv[])
{
	// Parse args
	if (argc <= 0)
		return;
	string id = argv[0];

	// Initialise the device
	hardware_mapping_t hmap = get_hardware_mapping(id);
	IntegratedDriveElectronicsController ide(hmap);

	// Wait for events
	DriverServer server(&ide);
	driver_ready(id);
	server.start(id);

}