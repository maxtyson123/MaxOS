//
// Created by 98max on 12/28/2022.
//

#include <filesystem/msdospart.h>

using namespace maxOS;
using namespace maxOS::drivers;
using namespace maxOS::common;
using namespace maxOS::filesystem;


// TODO: Get rid of printing as it isnt needed in the MBR? Maybe even make a debug stream for later usecaes like this
void printf(char* str, bool clearLine = false); // Forward declaration
void printfHex(uint8_t key);                    // Forward declaration

/**
 * @details Read the partition table of a given hard disk
 *
 * @param hd The hard disk to read the partition table from
 */
void MSDOSPartitionTable::ReadPartitions(AdvancedTechnologyAttachment *hd) {

    MasterBootRecord masterBootRecord;                                                           // Create a MasterBootRecord object

    hd -> Read28(0, (uint8_t*)&masterBootRecord, sizeof(MasterBootRecord));     // Read the MasterBootRecord from the disk

    /*
    printf("MBR: ");
    for (int i = 446; i < 466 + 4*16; ++i) {                                                     //Loop through the MBR and print it
        printfHex(((uint8_t*)&masterBootRecord)[i]);                                             //Print the buffer at the current index
        printf(" ");
    }
    printf("\n");
    */

    if(masterBootRecord.magicNumber != 0xAA55){                                                  // Check if the magic number is correct
        printf("Invalid MBR!");
        return;
    }

    for(int i = 0; i < 4; i++){                                                                  // Loop through the 4 primary partitions

        if(masterBootRecord.primaryPartition[i].partitionId == 0) continue;                      // If the partition id is 0, skip it

        printf("\nPartition ");
        printfHex(i & 0xFF);                                                                 // Print the partition number
        printf(": ");

        if(masterBootRecord.primaryPartition[i].bootable != 0x80) {                              // Check if the partition is bootable
            printf("Not ");
        }
        printf("Bootable, ");

        printf("Type ");
        printfHex(masterBootRecord.primaryPartition[i].partitionId);                         // Print the partition type
        printf("   ");


        //TODO: Add a message stream
        Fat32 fat32(hd, masterBootRecord.primaryPartition[i].startLBA, 0);              // Create a Fat32 object

    }
    printf("\n");

}
