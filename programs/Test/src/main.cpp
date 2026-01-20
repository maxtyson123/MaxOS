/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Init program
 *
 * @date 10th December 2025
 * @author Max Tyson
 */

#include <cstdint>
#include <processes/thread.h>
#include <string.h>

using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI;
using namespace MaxOS::common;

extern "C" void _start(int argc, char* argv[]){
	char* c = argv[0];

	// If B proc then wait a bit
	if (strcmp(c, "B")) {
		sleep(500);
	}

	while(true)
		klog( "T: %s\n", c);



}
