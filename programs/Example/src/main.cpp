//
// Created by 98max on 21/03/2025.
//
#include <stdint.h>
#include <stddef.h>
#include <ipc/messages.h>
#include <filesystem/file.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::filesystem;

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

	uint64_t fd = open_file("/test/a.txt");
	write("fd: \n");
	write_hex(fd);

	// Read Tests
// 	uint8_t buffer[50] = {};
//	file_read(fd, buffer, 50);
//	write("contents: \n");
//	write((char*)buffer);
//	write("%h\n");
//
//	write("offset: \n");
//	write_hex(file_offset(fd));
//
//	write("size: \n");
//	write_hex(file_size(fd));
//
//	write("new offset: \n");
//	seek_file(fd, 0, SeekType::SET);
//	write_hex(file_offset(fd));

	// Write tests
//	const char* message = "Heyyyy kernel";
//	write("writing: \n");
//	file_write(fd, (void*)message, strlen(message));
//
//	write("renaming: \n");
//	rename_file(fd, "b.txt");

	while (true)
		asm("nop");
}