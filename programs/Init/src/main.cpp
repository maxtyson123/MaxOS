/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Init program
 *
 * @date 10th December 2025
 * @author Max Tyson
 */

#include <cstdint>
#include <processes/thread.h>
#include <ipc/sharedmemory.h>
#include <libcommon/include/mem.h>
#include <server/fileserver_client.h>
#include <string.h>
#include <directory.h>
#include <file.h>
#include <path.h>

using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::KPI;
using namespace MaxOS;
using namespace MaxOS::common;
using namespace LibFS;

bool start_new(const entry_information_t* entry, const char* name) {
	klog("Starting initrd process %s\n", name);

	// Execute the file
	string path = (string)"/initrd/" + name;
	exec_file(Path::file_name(path).c_str(), path.c_str());

	return true;

}

extern "C" void _start(int argc, char* argv[]){

	// Parse the ram disk
	auto size	= (uint32_t)0xFFF000;
	auto header = (struct tar_header*)(0xFFF000 + sizeof(uint32_t));

	// Pass ramdisk to fileserver
	auto address = create_shared_memory("init_initrd", size);
	KPI::memcpy(address, header, size);
	mount_ramdisk("init_initrd");

	// Read the ramdisk
	klog("Opening initrd\n");
	auto handle = open_directory("/initrd");
	auto ent_size = directory_entries_size(handle);
	void* buffer = allocate_memory(ent_size);

	// Start driver manager & drivers
	directory_entries(handle, buffer, ent_size);
	iterate_entries(buffer, ent_size, start_new);

	// Wait for fileserver to parse disks

	// Start core services

	// Handoff to user login

	// Clean up
	resource_close(handle,0);
	free_memory(buffer);

	// Don't be scheduled again, but cant kms as that would kill child processes and thus core servers
	while(true)
		asm("nop");
		// sleep(UINT32_MAX);

}
