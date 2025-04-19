//
// Created by Max Tyson on 18/04/2025.
//

#ifndef MAXOS_FILESYSTEM_PARTITION_MSDOS_H
#define MAXOS_FILESYSTEM_PARTITION_MSDOS_H

#include <stdint.h>
#include <drivers/disk/disk.h>

namespace MaxOS{

  namespace filesystem{

    namespace partition{
        /**
         * @struct PartitionTableEntry
         * @brief Stores information about a partition
         */
        struct PartitionTableEntry{

          uint8_t     bootable;              // 0x80 = bootable, 0x00 = not bootable

          uint8_t     start_head;
          uint8_t     start_sector : 6;
          uint16_t    start_cylinder : 10;

          uint8_t     type;

          uint8_t     end_head;
          uint8_t     end_sector : 6;
          uint16_t    end_cylinder : 10;

          uint32_t    start_LBA;
          uint32_t    length;

        } __attribute__((packed));

        /**
         * @struct MasterBootRecord
         * @brief Stores information about the master boot record
         */
        struct MasterBootRecord{

          uint8_t     bootloader[440];
          uint32_t    disk_signature;
          uint16_t    unused;

          PartitionTableEntry primary_partition[4];

          uint16_t    magic;

        } __attribute__((packed));


        class MSDOSPartition
        {
          public:
            static void mount_partitions(drivers::disk::Disk* disk);
        };


        // TODO: Abstract some of this into a base class and use it for GPT and other partition tables



    }
  }
}

#endif //MSDOS_H
