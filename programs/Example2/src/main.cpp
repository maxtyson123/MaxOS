//
// Created by 98max on 21/03/2025.
//
#include <cstdint>
#include <cstddef>
#include <ipc/rpc.h>
#include <filesystem/directory.h>
#include <syscalls.h>

using namespace syscore;
using namespace syscore::ipc;
using namespace syscore::filesystem;

// Write using a syscall (int 0x80 with syscall 0x01 for write)
void write(const char* data)
{
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

void say_hi(const char* name)
{

	// Build RPC call
	ArgList args;
	args.push_string(name);

	// Make the call
	ArgList return_values;
	if(rpc_call("greeting_server", "say_hi", &args, &return_values))
	{
		write_hexa(return_values.arg_count());
		write_hexa(return_values.get_int64(0));
		write("RPC call suc!\n");
	} else{
		write("RPC call failed!\n");
	}
}

extern "C" void _start(void)
{
	// Write to the console
	write("MaxOS Test Program v3 - 2\n");

	thread_sleep(200);
	say_hi("MaxOS User");

	// Wait 0.5 seconds

	// Get lock

	// Write

	close();

}