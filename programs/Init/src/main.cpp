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

extern "C" void _start(int argc, char* argv[]){

	while(true)
		asm("nop");

	// Parse the ram disk
	klog("Loaded with %d args.\n", argc);
	for(int i = 0; i < argc; i++)
		klog("Arg %d: %s\n", i, argv[i]);

	// Pass to fileserver

	// Start driver manager

	// Wait for fileserver to parse disks

	// Start core servers

	// Handoff to user login

	// Don't be scheduled again, but cant kms as that would kill child processes and thus core servers
	while(true)
		sleep(UINT32_MAX);

}
