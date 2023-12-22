//
// Created by 98max on 12/28/2022.
//

#include <filesystem/msdospart.h>

using namespace maxOS;
using namespace maxOS::drivers;
using namespace maxOS::common;
using namespace maxOS::filesystem;

/**
 * @brief read the partition table of a given hard disk
 *
 * @param hd The hard disk to read the partition table from
 */
void MSDOSPartitionTable::read_partitions(drivers::AdvancedTechnologyAttachment *hd) {

    // read the MBR from the hard disk
    MasterBootRecord masterBootRecord;
    hd -> read_28(0, (uint8_t *)&masterBootRecord, sizeof(MasterBootRecord));

    // Check if the magic number is correct
    if(masterBootRecord.magicNumber != 0xAA55)
        return;


    // Loop through the primary partitions
    for(int i = 0; i < 4; i++){

        if(masterBootRecord.primaryPartition[i].partitionId == 0) continue;                      // If the partition id is 0, skip it

        //TODO: Add a message stream
        //TODO: Create a new FAT32 object

    }
}
