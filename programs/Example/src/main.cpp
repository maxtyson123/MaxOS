//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <ipc/rpc.h>
#include <filesystem/file.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::filesystem;

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data) {
	// don't care abt length for now
	asm volatile("int $0x80" : : "a" (0x01), "b" (data));
}


void close()
{
	// syscall 0, arg0 = pid (0 for current process), arg1 = exit code
	asm volatile(
			"mov $0, %%rdi\n\t"
			"mov $0, %%rsi\n\t"
			"mov $0x00, %%rax\n\t"
			"int $0x80\n\t"
			:
			:
			: "rax", "rdi", "rsi"
			);
}

int64_t say_hi_internal(const char* name)
{

	write("Hello, ");
	write(name);
	write("!\n");

	return 69;
}

void say_hi_wrapper(ArgList* args, ArgList* return_values){

	// Get args
	const char* name = args->get_string(0);

	// Call the internal function
	int64_t response = say_hi_internal(name);

	// Send response
	return_values->push_int64(response);


}

extern "C" void _start(void) {

	// Write to the console
	write("MaxOS Test Program v3 - 1\n");

	register_function("say_hi", say_hi_wrapper);
	rpc_server_loop("greeting_server");

	close();
}
