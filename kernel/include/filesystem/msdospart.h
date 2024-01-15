//
// Created by 98max on 12/28/2022.
//

#ifndef MAXOS_FILESYSTEM_MSDOSPART_H
#define MAXOS_FILESYSTEM_MSDOSPART_H

#include "drivers/disk/ata.h"
#include <filesystem/fat32.h>
#include <stdint.h>

namespace MaxOS{

    namespace filesystem{

        /**
         * @struct PartitionTableEntry
         * @brief Stores information about a partition
         */
        struct PartitionTableEntry{

            uint8_t     bootable;              // 0x80 = bootable, 0x00 = not bootable

            uint8_t     startHead;
            uint8_t     startSector : 6;
            uint16_t    startCylinder : 10;

            uint8_t     partitionId;

            uint8_t     endHead;
            uint8_t     endSector : 6;
            uint16_t    endCylinder : 10;

            uint32_t    startLBA;
            uint32_t    length;

        } __attribute__((packed));

        /**
         * @struct MasterBootRecord
         * @brief Stores information about the master boot record
         */
        struct MasterBootRecord{

            uint8_t     bootloader[440];
            uint32_t    diskSignature;
            uint16_t    unused;

            PartitionTableEntry primaryPartition[4];

            uint16_t    magicNumber;

        } __attribute__((packed));

        /**
         * @class MSDOSPartitionTable
         * @brief Reads the partition table of the hard drive
         */
        class MSDOSPartitionTable{
            public:
                static void read_partitions(drivers::disk::AdvancedTechnologyAttachment *hd);

        };
    }
}

#endif //MAXOS_FILESYSTEM_MSDOSPART_H
