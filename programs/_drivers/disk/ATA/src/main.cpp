//
// Created by Max Tyson on 14/04/2026.
//

#include <libdriver/device.h>
#include <libdriver/server/disk_server.h>
#include <libdriver/server/drivermanager_client.h>

#include <ata.h>

using namespace ATADriver;
using namespace MaxOS::KPI;
using namespace MaxOS;
using namespace LibDriver;
using namespace LibDriver::generic;
using namespace LibDriver::HardwareCommunication;

extern "C" void _start(int argc, char* argv[])
{

	// Fetch the device id
	if (argc <= 1)
		return;
	string id = argv[1];

	klog("ata driv %s\n", id.c_str());

	// Initialise the device
	hardware_mapping_t hmap = get_hardware_mapping(id);
	device_identification_t device = get_device_identification(id);
	AdvancedTechnologyAttachment ata(device, hmap);

	// Wait for events
	DiskServer server(&ata);
	driver_ready(id);
	server.start(id);
}