//
// Created by 98max on 12/28/2022.
//

#ifndef MAXOS_FILESYSTEM_MSDOSPART_H
#define MAXOS_FILESYSTEM_MSDOSPART_H

#include <common/types.h>
#include <drivers/ata.h>

namespace maxOS{

    namespace filesystem{

        struct PartitionTableEntry{

            common::uint8_t     bootable;              // 0x80 = bootable, 0x00 = not bootable

            common::uint8_t     startHead;             // starting head on the disk
            common::uint8_t     startSector : 6;       // starting sector on the disk
            common::uint16_t    startCylinder : 10;    // starting cylinder on the disk

            common::uint8_t     partitionId;           // partition type id

            common::uint8_t     endHead;               // ending head on the disk
            common::uint8_t     endSector : 6;         // ending sector on the disk
            common::uint16_t    endCylinder : 10;      // ending cylinder on the disk

            common::uint32_t    startLBA;              // starting LBA on the disk
            common::uint32_t    length;                // size of the partition in sectors

        } __attribute__((packed));

        struct MasterBootRecord{

            common::uint8_t     bootloader[440];       // bootloader code
            common::uint32_t    diskSignature;         // disk signature
            common::uint16_t    unused;                // unused

            PartitionTableEntry primaryPartition[4];   // primary partitions

            common::uint16_t    magicNumber;           // magic number

        } __attribute__((packed));

        class MSDOSPartitionTable{
            public:
                static void ReadPartitions(drivers::AdvancedTechnologyAttachment *hd);

        };

    }

}

#endif //MAXOS_FILESYSTEM_MSDOSPART_H
