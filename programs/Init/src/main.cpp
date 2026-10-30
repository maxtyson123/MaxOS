/**
 * @file main.cpp
 * @brief Entry point for the MaxOS Init program
 *
 * @date 10th December 2025
 * @author Max Tyson
 */

#include <cstdint>

#include <libkpi/processes/thread.h>
#include <libkpi/ipc/sharedmemory.h>
#include <libcommon/mem.h>
#include <libcommon/string.h>
#include <libfs/server/fileserver_client.h>
#include <libfs/directory.h>
#include <libfs/file.h>
#include <libfs/path.h>

using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::KPI;
using namespace MaxOS;
using namespace MaxOS::common;
using namespace LibFS;

static bool print_entry(const entry_information_t* entry, const char* name) {
	klog("FS TEST:   %s %s (%d bytes)\n", entry->is_file ? "file" : "dir ", name, (int) entry->size);
	return true;
}

static void filesystem_self_test() {

	const char* root      = "/1/";
	const char* file_name = "selftest.txt";
	const char* file_path = "/1/selftest.txt";

	klog("FS TEST: waiting for %s\n", root);
	uint64_t dir = 0;
	for (size_t tries = 0; tries < 20000 && !dir; tries++) {
		dir = open_directory(root);
		if (!dir)
			yield();
	}
	if (!dir) {
		klog("FS TEST: FAIL - %s never appeared\n", root);
		return;
	}

	size_t entries_size = directory_entries_size(dir);
	if (entries_size) {
		auto entries = new uint8_t[entries_size];
		directory_entries(dir, entries, entries_size);
		klog("FS TEST: %s contains:\n", root);
		iterate_entries(entries, entries_size, print_entry);
		delete[] entries;
	}

	remove_file(dir, file_name);
	new_file(dir, file_name);

	const size_t sizes[] = { 64, 6000 };
	bool all_passed = true;

	for (size_t size : sizes) {

		auto out = new uint8_t[size];
		auto in  = new uint8_t[size];
		for (size_t i = 0; i < size; i++) {
			out[i] = (uint8_t) ((i * 7 + size) & 0xFF);
			in[i]  = 0xEE;
		}

		// Write
		bool ok = true;
		uint64_t file = open_file(file_path);
		if (!file) {
			klog("FS TEST: FAIL - cant open %s\n", file_path);
			all_passed = false;
			delete[] out;
			delete[] in;
			continue;
		}
		seek_file(file, 0, SeekType::SET);
		file_write(file, out, size);
		if (file_size(file) != size) {
			klog("FS TEST: FAIL - file size is %d, expected %d\n", (int) file_size(file), (int) size);
			ok = false;
		}
		close_file(file);

		// Read it back
		file = open_file(file_path);
		if (file) {
			seek_file(file, 0, SeekType::SET);
			file_read(file, in, size);
			close_file(file);
		} else {
			ok = false;
		}

		// Compare
		for (size_t i = 0; i < size && ok; i++) {
			if (in[i] != out[i]) {
				klog("FS TEST: FAIL - mismatch at byte %d (wrote %x, read %x)\n", (int) i, (uint64_t) out[i], (uint64_t) in[i]);
				ok = false;
			}
		}

		klog("FS TEST: write/read %d bytes - %s\n", (int) size, ok ? "PASS" : "FAIL");
		all_passed = all_passed && ok;

		delete[] out;
		delete[] in;
	}

	// Clean up the test file
	remove_file(dir, file_name);
	close_directory(dir);

	klog("FS TEST: %s\n", all_passed ? "ALL PASSED" : "FAILED");
}

extern "C" void _start(int argc, char* argv[])
{
	// Parse the ram disk
    klog("welcome to userspace\n");
    auto size = *(volatile uint32_t*)0xFFF000;
	auto header = (struct tar_header*)(0xFFF000 + sizeof(uint32_t));

	// Pass ramdisk to fileserver
    auto address = create_shared_memory("init_initrd", size);
    if (!address) {
        klog("Init: initrd region FAILED\n");
        exit(1);
    }

	memcpy(address, header, size);
    wait_for_fileserver_server();
	mount_ramdisk("init_initrd");
    klog("ramdisk mounted\n");

	// Start driver manager
	string dm_path = "/0/boot/initrd/DriverManager.elf";
	exec_file(Path::file_name(dm_path).c_str(), dm_path.c_str());
    klog("DriverManager launched\n");

	// Wait for fileserver to parse disks
    filesystem_self_test();

	// Start core services

	// Handoff to user login

	// Don't be scheduled again, but cant kms as that would kill child processes and thus core servers
	while (true)
	    sleep(UINT32_MAX);
}
