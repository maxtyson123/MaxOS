//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <ipc/rpc.h>
#include <greeting/greeter_server.h>
#include <filesystem/file.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::filesystem;


int64_t say_hi(const char* name)
{

	klog("Hello, ");
	klog(name);
	klog("!\n");
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
	klog("Notification received: ");
	klog(message);
	klog("\n");
}

extern "C" void _start(void) {

	// Write to the console
	klog("MaxOS Test Program v3 - 1\n");

	run_greeter();
}
