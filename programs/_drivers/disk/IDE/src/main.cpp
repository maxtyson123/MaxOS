//
// Created by Max Tyson on 14/04/2026.
//

#include <libdriver/device.h>
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

    klog("HI from ide driver\n");

	// Fetch the device id
	if (argc <= 1)
		return;
	string id = argv[1];

	// Initialise the device
	hardware_mapping_t hmap = get_hardware_mapping(id);
	device_identification_t device = get_device_identification(id);
	IntegratedDriveElectronicsController ide(device, hmap);

	// Wait for events
	DriverServer server(&ide);
	driver_ready(id);
	server.start(id);
}