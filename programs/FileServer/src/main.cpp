/**
 * @file main.cpp
 * @brief Entry point for the MaxOS FileServer program
 *
 * @date 21st Janurary 2026
 * @author Max Tyson
 */

#include <cstdint>
#include <processes/thread.h>
#include <string.h>
#include <libfs/include/server/fileserver_server.h>
#include <vfsresource.h>

using namespace MaxOS::KPI;
using namespace MaxOS::KPI::processes;
using namespace MaxOS::KPI::ipc;
using namespace MaxOS::common;
using namespace FileServer;
using namespace LibFS;

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
        if (!vfs_resources.queue_empty() && !did_handle) {
            yield();
            continue;
        }
        // If here is reached, either there is a resource call to handle OR a rpc call was handled and there may be a
        // message waiting on the queue, cannot know until next loop OR both.

        // Handle resource calls
        vfs_resources.process_next();

    }


}
