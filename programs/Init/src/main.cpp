/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Init program
 *
 * @date 10th December 2025
 * @author Max Tyson
 */

#include <cstdint>
#include <processes/thread.h>

using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI;

struct tar_header
{
	char filename[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag[1];
};

extern "C" void _start(int argc, char* argv[]){

	klog("hi\n");

	// Parse the ram disk
	auto header = (struct tar_header*)0xFFF000;
	klog(header->filename);

	// Pass to fileserver

	// Start driver manager

	// Wait for fileserver to parse disks

	// Start core services

	// Handoff to user login

	// Don't be scheduled again, but cant kms as that would kill child processes and thus core servers
	while(true)
		sleep(UINT32_MAX);

}
