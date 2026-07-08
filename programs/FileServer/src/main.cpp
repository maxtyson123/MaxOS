/**
 * @file main.cpp
 * @brief Entry point for the MaxOS FileServer program
 *
 * @date 21st Janurary 2026
 * @author Max Tyson
 */

#include <cstdint>

#include <libkpi/processes/thread.h>
#include <libcommon/string.h>
#include <libfs/server/fileserver_server.h>
#include <libkpi/ipc/sharedmemory.h>
#include <libkpi/syscalls.h>
#include <libdriver/server/drivermanager_client.h>

#include <format/tar.h>
#include <vfs/vfsresource.h>
#include <filesystem/fs_event_handler.h>

using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::common;
using namespace FileServer;
using namespace FileServer::format;
using namespace FileServer::filesystem;
using namespace FileServer::vfs;
using namespace LibFS;

void mount_ramdisk(mstring endpoint) {

    // Try oad the initrd
    void* address = open_shared_memory(endpoint.c_str());
    if (!address)
        return;

    // Parse the initrd (@todo verify and shit)
    auto fs = new TARFileSystem(address);

    // Mount
    auto vfs = VirtualFileSystem::current_file_system();
    if (!vfs)
        return;


    vfs->mount_filesystem(fs, "/0/boot/initrd");
}

extern "C" void _start(int argc, char* argv[]){

    klog("Setting up FileServer\n");

    // Set up the virtual filesystem
    VirtualFileSystem vfs;

    // Setup the servers
    VFSResourceServer vfs_resources(&vfs);
    uint64_t handle = register_fileserver();
	FSDriverManagerEventHanlder fs_driver_manager_event_hanlder;

    while(true) {

        // Try to handle any rpc call
    	bool did_main_server = rpc_server_process_next(handle, false);
    	bool did_event_server = fs_driver_manager_event_hanlder.process_next();

    	// Handle resource calls
    	bool did_resource = vfs_resources.process_next();

    	// Try register for events (need driver manager to start)
    	fs_driver_manager_event_hanlder.try_setup_self();

        // Check if there is anything to do
    	bool did_work = did_main_server || did_event_server || did_resource;
        if (!did_work)
        	yield();
    }
}