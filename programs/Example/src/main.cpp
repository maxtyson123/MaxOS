//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <ipc/rpc.h>
#include <greeting/greeter_server.h>
#include <filesystem/file.h>
#include <mem.h>


using namespace MaxOS::common;
using namespace MaxOS::KPI;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::KPI::filesystem;

#include <string.h>


int64_t say_hi(const char* name)
{

	klog("Hello, %s! Welcome to MaxOS.\n", name);
	return 0;
}

compute_answer_result_t compute_answer(uint32_t a, uint32_t b)
{
	compute_answer_result_t result;
	result.v0 = a + b;
	result.v1 = "Computation complete.";
	return result;
}

void notify(const char* message)
{
	klog("Notification received: %s\n", message);
}

extern "C" void _start(void) {

	// Write to the console
	klog("MaxOS Test Program v3 - 1\n");

	// Simple buffer malloc test
	void* buffer = new uint8_t[256];
	klog("Allocated buffer at address: 0x%x\n", buffer);
	memset(buffer, 0xAB, 256);
	delete[] buffer;



	run_greeter();
}
