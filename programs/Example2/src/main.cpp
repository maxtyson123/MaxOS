//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <greeting/greeter_client.h>
#include <processes/process.h>
#include <syscalls.h>

using namespace MaxOS::KPI;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::KPI::processes;

extern "C" void _start(void)
{
	// Write to the console
	klog("MaxOS Test Program v3 - 2\n");
	wait_for_greeter_server();

	say_hi("MaxOS User");

	compute_answer_result_t answer = compute_answer(42, 100);
	klog("Computed Answer Result: %d with message '%s'\n", answer.v0, answer.v1);

	notify("This is a notification from the greeter client.");

	exit(0);
}