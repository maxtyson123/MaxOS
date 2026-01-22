/**
 * @file msdos.cpp
 * @brief Implementation of MSDOS partition table reading and mounting
 *
 * @date 28th December 2022
 * @author Max Tyson
 */

#include <partition/msdos.h>

using namespace FileServer;
using namespace FileServer::format;
using namespace FileServer::format::ext2;
using namespace FileServer::partition;
using namespace LibDriver::generic;
using namespace MaxOS::KPI;

/**
 * @brief read the partition table of a given hard disk
 *
 * @param disk The hard disk to read the partition table from
 */
void MSDOSPartition::mount_partitions(Disk* disk) {

	// Read the MBR from the hard disk
	MasterBootRecord mbr = {};
	buffer_t mbr_buffer(&mbr, sizeof(MasterBootRecord));
	disk->read(0, &mbr_buffer);

	// Check if the magic number is correct
	if (mbr.magic != 0xAA55) {
		klog("Could not find valid MBR on disk 0x%x\n", disk);
		return;
	}

	// Get the VFS
	VirtualFileSystem *vfs = VirtualFileSystem::current_file_system();

	// Loop through the primary partitions
	for (auto &entry: mbr.primary_partition) {

		// Empty entry
		if (entry.type == 0)
			continue;

		// Create a file system for the partition
		switch ((PartitionType) entry.type) {
			case PartitionType::EMPTY:
				klog("Empty partition\n");
				break;

			case PartitionType::FAT32:
				vfs->mount_filesystem(new Fat32FileSystem(disk, entry.start_LBA));
				break;

			case PartitionType::LINUX_EXT2:
				vfs->mount_filesystem(new ext2::Ext2FileSystem(disk, entry.start_LBA));
				break;

			default:
				klog("Unknown or unimplemented partition type: 0x%x\n", (uint64_t) entry.type );
		}
	}
}
