//
// Created by 98max on 12/28/2022.
//

#include <filesystem/partition/msdos.h>

using namespace MaxOS;
using namespace MaxOS::filesystem;
using namespace MaxOS::filesystem::partition;
using namespace MaxOS::drivers::disk;

/**
 * @brief read the partition table of a given hard disk
 *
 * @param hd The hard disk to read the partition table from
 */
void MSDOSPartition::mount_partitions(Disk* hd) {

  // Read the MBR from the hard disk
  MasterBootRecord mbr = {};
  hd -> read(0, (uint8_t *)&mbr, sizeof(MasterBootRecord));

  // Check if the magic number is correct
  if(mbr.magic != 0xAA55)
  {
    Logger::WARNING() << "Could not find valid MBR on disk 0x" << (uint64_t)hd << "\n";
    return;
  }

  // Get the VFS
  VirtualFileSystem* vfs = VirtualFileSystem::current_file_system();

  // Loop through the primary partitions
  for(auto& entry : mbr.primary_partition){

    // Empty entry
    if(entry.type == 0)
      continue;

    Logger::DEBUG() << "Partition 0x" << (uint64_t)entry.type << " at 0x" << (uint64_t)entry.start_LBA << ": ";

    // Create a file system for the partition
    switch ((PartitionType)entry.type)
    {
        case PartitionType::EMPTY:
          Logger::Out() << "Empty partition\n";
          break;

        case PartitionType::FAT32:
          Logger::Out() << "FAT32 partition\n";
          vfs -> mount_filesystem(new Fat32FileSystem(hd, entry.start_LBA));
          break;

        default:
          Logger::Out() << "Unknown or unimplemented partition type: 0x" << (uint64_t)entry.type << "\n";

    }
  }
}
