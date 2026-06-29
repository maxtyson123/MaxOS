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

#include <format/tar.h>
#include <vfsresource.h>

using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::common;
using namespace FileServer;
using namespace FileServer::format;
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

    while(true) {

        // Try to handle any rpc call
        bool did_handle = rpc_server_process_next(handle, false);

        // Check if there is anything to do
        if (vfs_resources.queue_empty() && !did_handle) {
            yield();
            continue;
        }
        // If here is reached, either there is a resource call to handle OR a rpc call was handled and there may be a
        // message waiting on the queue, cannot know until next loop OR both.

        // Handle resource calls
        vfs_resources.process_next();
    }
}
