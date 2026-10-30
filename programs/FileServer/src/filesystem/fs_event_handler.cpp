//
// Created by 98max on 1/07/2026.
//

#include <filesystem/fs_event_handler.h>

using namespace LibDriver;
using namespace LibDriver::generic;
using namespace MaxOS::KPI;
using namespace FileServer;
using namespace FileServer::filesystem;
using namespace FileServer::partition;

FSDriverManagerEventHanlder::FSDriverManagerEventHanlder() {
}

FSDriverManagerEventHanlder::~FSDriverManagerEventHanlder() = default;

void FSDriverManagerEventHanlder::on_driver_ready(mstring id) {

    //  Only care about disks (@todo only sub to disks?)
    device_identification_t info =  get_device_identification(id);
    if (info.driver_type != DriverType::DISK)
        return;

    // Discover partitions on disk
    auto disk = new DiskClient(id);
    MSDOSPartition::mount_partitions(disk);

	klog("added disk with device id=%s\n", id.c_str());
}


