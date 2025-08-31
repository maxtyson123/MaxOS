//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>
#include <stddef.h>
#include <ipc/messages.h>

using namespace IPC;

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data) {
	// don't care abt length for now
	asm volatile("int $0x80" : : "a" (0x01), "b" (data));
}

void write_hex(uint64_t data) {
	// Convert to hex
	char buffer[20];
	buffer[0] = '0';
	buffer[1] = 'x';
	buffer[18] = '\n';
	buffer[19] = '\0';

	// Convert to hex
	for (int i = 0; i < 16; i++) {
		// Get the current nibble
		uint8_t nibble = (data >> (60 - i * 4)) & 0xF;

		// Convert to hex
		buffer[2 + i] = nibble < 10 ? '0' + nibble : 'A' + nibble - 10;
	}

	// Write the hex
	write(buffer);
}

void yield() {

	asm volatile("int $0x80" : : "a" (0x09));
}

extern "C" [[noreturn]] void _start(void) {

	// Write to the console
	write("MaxOS Test Program v3\n");

	// Create a message endpoint
	uint64_t queue = create_endpoint("TestQueue");
	if (!queue) {
		write("Failed to create message queue\n");
		while (true)
			yield();
	}

	write("Message queue created: \n");
	write_hex(queue);

	// Process events forever:
	write("Waiting for messages\n");
	uint8_t message[255] = {};

	while (true){

		// Store the message
		read_message(queue, message, 255);
		write("Message received: \n");
		write((char*)message);

	}
}