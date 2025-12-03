//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <greeting/greeter_client.h>
#include <processes/process.h>
#include <syscalls.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::processes;


void write_hexa(uint64_t value) {

	const char* hex_chars = "0123456789ABCDEF";
	char buffer[17];
	for (int i = 0; i < 16; ++i) {
		buffer[15 - i] = hex_chars[value & 0xF];
		value >>= 4;
	}
	buffer[16] = '\0';
	klog(buffer);
}

extern "C" void _start(void)
{
	// Write to the console
	klog("MaxOS Test Program v3 - 2\n");

	say_hi("MaxOS User");

	compute_answer_result_t answer = compute_answer(42, 100);
	klog("The answer is: ");
	write_hexa(answer.v0);
	klog("%h\n");
	klog("With message: ");
	klog(answer.v1);
	klog("%h\n");
	while (true){ asm("nop"); }

	notify("This is a notification from the greeter client.");


	exit(0);

	//TODO: exit() broken, notfiy broken, RPC call ret not wokring

}