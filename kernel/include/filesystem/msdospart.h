//
// Created by 98max on 12/28/2022.
//

#ifndef MAXOS_FILESYSTEM_MSDOSPART_H
#define MAXOS_FILESYSTEM_MSDOSPART_H

#include <stdint.h>
#include <drivers/ata.h>
#include <filesystem/fat32.h>


namespace maxOS{

    namespace filesystem{

        struct PartitionTableEntry{

            uint8_t     bootable;              // 0x80 = bootable, 0x00 = not bootable

            uint8_t     startHead;             // starting head on the disk
            uint8_t     startSector : 6;       // starting sector on the disk
            uint16_t    startCylinder : 10;    // starting cylinder on the disk

            uint8_t     partitionId;           // partition type id

            uint8_t     endHead;               // ending head on the disk
            uint8_t     endSector : 6;         // ending sector on the disk
            uint16_t    endCylinder : 10;      // ending cylinder on the disk

            uint32_t    startLBA;              // starting LBA on the disk
            uint32_t    length;                // size of the partition in sectors

        } __attribute__((packed));

        struct MasterBootRecord{

            uint8_t     bootloader[440];       // bootloader code
            uint32_t    diskSignature;         // disk signature
            uint16_t    unused;                // unused

            PartitionTableEntry primaryPartition[4];   // primary partitions

            uint16_t    magicNumber;           // magic number

        } __attribute__((packed));

        class MSDOSPartitionTable{
            public:
                static void ReadPartitions(drivers::AdvancedTechnologyAttachment *hd);

        };

    }

}

#endif //MAXOS_FILESYSTEM_MSDOSPART_H
